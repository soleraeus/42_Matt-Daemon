/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/28 20:21:03 by bdetune           #+#    #+#             */
/*   Updated: 2023/10/23 20:01:48 by bdetune          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(void): _fd(0), _packetsize(0), _secure(false), _handshake(false), _key(NULL){}
Client::Client(int fd, bool secure, unsigned char* key): _fd(fd), _packetsize(0), _secure(secure), _handshake(false), _key(key){
	if (secure)
	{
		if (!RAND_bytes(this->_iv, 16))
		{
			throw std::system_error(std::error_code(), "Could not generate iv");
		}
	}	
}

Client::Client(const Client & src): _fd(dup(src._fd)), _packetsize(src._packetsize), _secure(src._secure), _handshake(src._handshake), _key(src._key), _buffer(src._buffer), _encrypted_buffer(src._encrypted_buffer) {
	memcpy(this->_iv, src._iv, 16);
	memcpy(this->_RSA_buf, src._RSA_buf, 1024);
}
Client::Client(Client && src): _fd(std::move(src._fd)), _packetsize(src._packetsize), _secure(src._secure), _handshake(src._handshake), _key(src._key), _buffer(std::move(src._buffer)), _encrypted_buffer(std::move(src._encrypted_buffer)) {
	memcpy(this->_iv, src._iv, 16);
	memcpy(this->_RSA_buf, src._RSA_buf, 1024);
	src._fd = 0;
	src._key = NULL;
}

Client::~Client(void) {
	if (this->_fd > 0)
		close(this->_fd);
}

Client & Client::operator=(const Client & rhs)
{
	if (this == &rhs)
		return (*this);
	this->_fd = dup(rhs._fd);
	this->_packetsize = rhs._packetsize;
	this->_secure = rhs._secure;
	this->_handshake = rhs._handshake;
	this->_buffer = rhs._buffer;
	this->_encrypted_buffer = rhs._encrypted_buffer;
	this->_key = rhs._key;
	memcpy(this->_iv, rhs._iv, 16);
	memcpy(this->_RSA_buf, rhs._RSA_buf, 1024);
	return (*this);
}

Client & Client::operator=(Client && rhs)
{
	if (this == &rhs)
		return (*this);
	this->_fd = rhs._fd;
	rhs._fd = 0;
	this->_packetsize = rhs._packetsize;
	this->_secure = rhs._secure;
	this->_handshake = rhs._handshake;
	this->_buffer = std::move(rhs._buffer);
	this->_encrypted_buffer = std::move(rhs._encrypted_buffer);
	this->_key = rhs._key;
	rhs._key = NULL;
	memcpy(this->_iv, rhs._iv, 16);
	memcpy(this->_RSA_buf, rhs._RSA_buf, 1024);
	return (*this);
}

Client::Return	Client::getPacketSize(void)
{
	size_t	pos = 0;

	this->_packetsize = 0;
	for (size_t i = 0; i < this->_encrypted_buffer.size(); ++i)
	{
		if (this->_encrypted_buffer[i] == '\n')
		{
			if (i < 8 || i > 12)
				return Client::Return::KICK;
			if (memcmp((void *)"Length ",(void *)&this->_encrypted_buffer[0], 7))
				return Client::Return::KICK;
			pos = i;
			if (this->_encrypted_buffer[i - 1] == '\r')
				pos = i - 1;
			if (pos == 7)
				return Client::Return::KICK;
			this->_encrypted_buffer[pos] = '\0';
			for (size_t j = 7; j < pos; ++j)
			{
				if (this->_encrypted_buffer[j] < '0' || this->_encrypted_buffer[j] > '9')
					return Client::Return::KICK;
			}
			this->_packetsize = std::atoi(&this->_encrypted_buffer[7]);
			if (this->_packetsize > PIPE_BUF)
				return Client::Return::KICK;
			this->_encrypted_buffer.erase(this->_encrypted_buffer.begin(), this->_encrypted_buffer.begin() + i + 1);
			return Client::Return::OK;
		}
		if ((this->_encrypted_buffer[i] < ' ' || this->_encrypted_buffer[i] > 'z')
            && this->_encrypted_buffer[i] != '\r') {
			return Client::Return::KICK;
		}
	}
	if (this->_encrypted_buffer.size() > 12)
		return Client::Return::KICK;
	return Client::Return::OK;
}

Client::Return	Client::flush(std::shared_ptr<Tintin_reporter>& reporter)
{
	std::string				sub;
	std::string::size_type	pos;

	while ((pos = this->_buffer.find('\r')) != std::string::npos)
	{
		if (pos == this->_buffer.size() - 1)
			break;
		if (this->_buffer[pos + 1] != '\n')
			return Client::Return::KICK;
		this->_buffer.erase(pos, 1);
	}
	while ((pos = this->_buffer.find('\n')) != std::string::npos)
	{
		sub = this->_buffer.substr(0, pos);
		if (sub == "quit")
			return Client::Return::QUIT;
		if (reporter->client_log(sub) != Tintin_reporter::Return::OK)
			return Client::Return::QUIT;
		this->_buffer.erase(0, pos + 1);
	}
	if (this->_buffer.size() > PIPE_BUF)
		return Client::Return::KICK;
	return Client::Return::OK;
}

Client::Return	Client::receive(std::shared_ptr<Tintin_reporter>& reporter)
{
	ssize_t					len = 0;
	Client::Return			ret;
	
	len = recv(this->_fd, (void *)this->_recv_buffer, PIPE_BUF, MSG_DONTWAIT);
	if (len <= 0)
		return Client::Return::KICK;
	if (this->_secure)
	{
		this->_encrypted_buffer.insert(this->_encrypted_buffer.end(), this->_recv_buffer, &this->_recv_buffer[len]);
		if (!this->_packetsize && (ret = this->getPacketSize()) != Client::Return::OK)
			return ret;
		while (this->_packetsize > 0 && this->_encrypted_buffer.size() >= static_cast<size_t>(this->_packetsize))
		{
			if (!this->_handshake) {
				if (this->_packetsize > 1023)
					return Client::Return::KICK;
				memcpy(this->_RSA_buf, this->_encrypted_buffer.data(), this->_packetsize);
				this->_RSA_buf[this->_packetsize] = '\0';
				this->_encrypted_buffer.erase(this->_encrypted_buffer.begin(), this->_encrypted_buffer.begin() + this->_packetsize);
				return (this->sendKey());
			}
			std::cerr << "It would be time to decrypt" << std::endl;
			this->_buffer.insert(this->_buffer.end(), this->_encrypted_buffer.begin(), this->_encrypted_buffer.begin() + this->_packetsize);
			this->_encrypted_buffer.erase(this->_encrypted_buffer.begin(), this->_encrypted_buffer.begin() + this->_packetsize);
			if ((ret = this->getPacketSize()) != Client::Return::OK)
				return ret;
		}
	}
	else
		this->_buffer.insert(this->_buffer.end(), this->_recv_buffer, &this->_recv_buffer[len]);
	return (this->flush(reporter));
}

Client::Return	Client::sendKey(void) {
    size_t  outlen;

	//Putting public key received in BIO
	BIO*		pubKey = BIO_new_mem_buf((void*) this->_RSA_buf, this->_packetsize);
	if (!pubKey)
		return Client::Return::KICK;

	this->_packetsize = 0;
	//Using the BIO to create an EVP_KEY
	EVP_PKEY* RSA_key = NULL;
	RSA_key	= PEM_read_bio_PUBKEY(pubKey, &RSA_key, NULL, NULL);
	if (!RSA_key) {
		std::cerr << "Could not create RSA key" << std::endl;
		BIO_free(pubKey);
		return Client::Return::KICK;
	}
    EVP_PKEY_CTX*   ctx = EVP_PKEY_CTX_new(RSA_key, NULL);
    if (!ctx) {
      std::cerr << "Could not create context" << std::endl;
      BIO_free(pubKey);
      EVP_PKEY_free(RSA_key);
      return Client::Return::KICK;
    }
    if (EVP_PKEY_encrypt_init(ctx) <= 0) {
      EVP_PKEY_CTX_free(ctx);
      EVP_PKEY_free(RSA_key);
      BIO_free(pubKey);
      std::cerr << "Could not init encryption" <<std::endl;
      return Client::Return::KICK;
    }
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
      EVP_PKEY_CTX_free(ctx);
      EVP_PKEY_free(RSA_key);
      BIO_free(pubKey);
      std::cerr << "Could not set padding" <<std::endl;
      return Client::Return::KICK;
    }
    std::string out;
    out.assign(this->_key, this->_key + 32);
    out.insert(out.end(), this->_iv, this->_iv + 16);
    if (EVP_PKEY_encrypt(ctx, nullptr, &outlen, (const unsigned char*) out.data(), out.size()) <= 0) {
      std::cerr << "Could not get length of encrypted result" << std::endl;
      EVP_PKEY_CTX_free(ctx);
      EVP_PKEY_free(RSA_key);
      BIO_free(pubKey);
      return Client::Return::KICK;
    }
    std::cerr << "Payload is " << outlen << " bytes" << std::endl;
    unsigned char* out_tmp_buf = NULL; 
    try {
      out_tmp_buf = new unsigned char[outlen];
    }
    catch (std::exception const & e) {
      std::cerr << "Allocation error" << std::endl;
      EVP_PKEY_CTX_free(ctx);
      EVP_PKEY_free(RSA_key);
      BIO_free(pubKey);
      return Client::Return::KICK;
    }
    if (EVP_PKEY_encrypt(ctx, out_tmp_buf, &outlen, (const unsigned char*) out.data(), out.size()) <= 0) {
      std::cerr << "Could encrypt buffer" << std::endl;
      delete [] out_tmp_buf;
      EVP_PKEY_CTX_free(ctx);
      EVP_PKEY_free(RSA_key);
      BIO_free(pubKey);
      return Client::Return::KICK;
    }
    this->_send_buffer = "Length ";
    this->_send_buffer += std::to_string(outlen);
    this->_send_buffer += "\n";
    std::cerr << "Length: " << outlen << std::endl;
    this->_send_buffer.insert(this->_send_buffer.end(), out_tmp_buf, out_tmp_buf + outlen);
    std::cerr << "Original key" << std::endl;
    for (int i = 0; i < 32; ++i) {
      std::cerr << std::setw(2) << std::setfill('0') << std::hex << (int)this->_key[i];
    }
    std::cerr << std::endl << "Original iv" << std::endl;
    for (int i = 0; i < 16; ++i) {
      std::cerr << std::setw(2) << std::setfill('0') << std::hex << (int)this->_iv[i];
    }
    std::cerr << std::endl << "Encrypted" << std::endl;
    for (size_t i = this->_send_buffer.find('\n') + 1; i < this->_send_buffer.size(); ++i) {
      std::cerr << std::setw(2) << std::setfill('0') << std::hex << (int)this->_send_buffer[i];
    }
    std::cerr << std::endl << std::dec << "Final length: " << this->_send_buffer.size() << std::endl;
    delete [] out_tmp_buf;
    EVP_PKEY_CTX_free(ctx);
	EVP_PKEY_free(RSA_key);
	BIO_free(pubKey);
	std::cerr << "Succesfully loaded public key received" << std::endl;
	return Client::Return::SEND;
}

std::string&    Client::getSendBuffer(void) {
  return this->_send_buffer;
}
