/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/28 20:21:03 by bdetune           #+#    #+#             */
/*   Updated: 2023/10/24 21:55:55 by bdetune          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(void): _fd(0), _packetsize(0), _secure(false), _handshake(false), _key(nullptr), _ctx(nullptr), _cipher(nullptr) {}
Client::Client(int fd, bool secure, unsigned char* key): _fd(fd), _packetsize(0), _secure(secure), _handshake(false), _key(key), _ctx(nullptr), _cipher(nullptr) {
    if (secure)
    {
        if (!RAND_bytes(this->_iv, 16))
        {
            throw std::system_error(std::error_code(), "Could not generate iv");
        }
        if ((_ctx = EVP_CIPHER_CTX_new()) == nullptr) {
            throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not create new context");
        }
        if ((_cipher = EVP_CIPHER_fetch(nullptr, "AES-256-GCM", nullptr)) == nullptr) {
            EVP_CIPHER_CTX_free(_ctx);
            throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not fetch cipher");
        }
    }   
}

Client::Client(const Client & src): _fd(dup(src._fd)), _packetsize(src._packetsize), _secure(src._secure), _handshake(src._handshake), _key(src._key), _buffer(src._buffer), _encrypted_buffer(src._encrypted_buffer) {
    memcpy(this->_iv, src._iv, 16);
    memcpy(this->_RSA_buf, src._RSA_buf, 1024);
    if (src._secure) {
        if ((_ctx = EVP_CIPHER_CTX_new()) == nullptr) {
            throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not create new context");
        }
        if ((_cipher = EVP_CIPHER_fetch(nullptr, "AES-256-GCM", nullptr)) == nullptr) {
            EVP_CIPHER_CTX_free(_ctx);
            throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not fetch cipher");
        }
    }
}
Client::Client(Client && src): _fd(std::move(src._fd)), _packetsize(src._packetsize), _secure(src._secure), _handshake(src._handshake), _key(src._key), _buffer(std::move(src._buffer)), _encrypted_buffer(std::move(src._encrypted_buffer)), _ctx(src._ctx), _cipher(src._cipher) {
    memcpy(this->_iv, src._iv, 16);
    memcpy(this->_RSA_buf, src._RSA_buf, 1024);
    src._fd = 0;
    src._key = nullptr;
    src._ctx = nullptr;
    src._cipher = nullptr;
}

Client::~Client(void) {
    if (this->_fd > 0)
        close(this->_fd);
    if (this->_ctx)
        EVP_CIPHER_CTX_free(this->_ctx);
    if (this->_cipher)
        EVP_CIPHER_free(_cipher);
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
    if (rhs._secure) {
        if ((_ctx = EVP_CIPHER_CTX_new()) == nullptr) {
            throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not create new context");
        }
        if ((_cipher = EVP_CIPHER_fetch(nullptr, "AES-256-GCM", nullptr)) == nullptr) {
            EVP_CIPHER_CTX_free(_ctx);
            throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not fetch cipher");
        }
    }
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
    rhs._key = nullptr;
    memcpy(this->_iv, rhs._iv, 16);
    memcpy(this->_RSA_buf, rhs._RSA_buf, 1024);
    this->_ctx = rhs._ctx;
    rhs._ctx = nullptr;
    this->_cipher = rhs._cipher;
    rhs._cipher = nullptr;
    return (*this);
}

Client::Return  Client::getPacketSize(void)
{
    size_t  pos = 0;

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
            if (this->_packetsize > PIPE_BUF + 128 + 16)
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

Client::Return  Client::flush(std::shared_ptr<Tintin_reporter>& reporter)
{
    std::string             sub;
    std::string::size_type  pos;

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
        if (pos > PIPE_BUF)
            return Client::Return::KICK;
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

Client::Return  Client::receive(std::shared_ptr<Tintin_reporter>& reporter)
{
    ssize_t                 len = 0;
    Client::Return          ret;
    
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
            if (!this->decrypt())
                return Client::Return::KICK;
           // this->increaseIV();
            if ((ret = this->getPacketSize()) != Client::Return::OK)
                return ret;
        }
    }
    else
        this->_buffer.insert(this->_buffer.end(), this->_recv_buffer, &this->_recv_buffer[len]);
    return (this->flush(reporter));
}

Client::Return  Client::sendKey(void) {
    size_t  outlen;

    //Putting public key received in BIO
    BIO*        pubKey = BIO_new_mem_buf((void*) this->_RSA_buf, this->_packetsize);
    if (!pubKey)
        return Client::Return::KICK;

    this->_packetsize = 0;
    //Using the BIO to create an EVP_KEY
    EVP_PKEY* RSA_key = nullptr;
    RSA_key = PEM_read_bio_PUBKEY(pubKey, &RSA_key, nullptr, nullptr);
    if (!RSA_key) {
        BIO_free(pubKey);
        return Client::Return::KICK;
    }
    EVP_PKEY_CTX*   ctx = EVP_PKEY_CTX_new(RSA_key, nullptr);
    if (!ctx) {
      BIO_free(pubKey);
      EVP_PKEY_free(RSA_key);
      return Client::Return::KICK;
    }
    if (EVP_PKEY_encrypt_init(ctx) <= 0) {
      EVP_PKEY_CTX_free(ctx);
      EVP_PKEY_free(RSA_key);
      BIO_free(pubKey);
      return Client::Return::KICK;
    }
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
      EVP_PKEY_CTX_free(ctx);
      EVP_PKEY_free(RSA_key);
      BIO_free(pubKey);
      return Client::Return::KICK;
    }
    std::string out;
    out.assign(this->_key, this->_key + 32);
    out.insert(out.end(), this->_iv, this->_iv + 16);
    if (EVP_PKEY_encrypt(ctx, nullptr, &outlen, (const unsigned char*) out.data(), out.size()) <= 0) {
      EVP_PKEY_CTX_free(ctx);
      EVP_PKEY_free(RSA_key);
      BIO_free(pubKey);
      return Client::Return::KICK;
    }
    unsigned char* out_tmp_buf = nullptr; 
    try {
      out_tmp_buf = new unsigned char[outlen];
    }
    catch (std::exception const & e) {
      EVP_PKEY_CTX_free(ctx);
      EVP_PKEY_free(RSA_key);
      BIO_free(pubKey);
      return Client::Return::KICK;
    }
    if (EVP_PKEY_encrypt(ctx, out_tmp_buf, &outlen, (const unsigned char*) out.data(), out.size()) <= 0) {
      delete [] out_tmp_buf;
      EVP_PKEY_CTX_free(ctx);
      EVP_PKEY_free(RSA_key);
      BIO_free(pubKey);
      return Client::Return::KICK;
    }
    this->_send_buffer = "Length ";
    this->_send_buffer += std::to_string(outlen);
    this->_send_buffer += "\n";
    this->_send_buffer.insert(this->_send_buffer.end(), out_tmp_buf, out_tmp_buf + outlen);
    delete [] out_tmp_buf;
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(RSA_key);
    BIO_free(pubKey);
    this->_handshake = true;
    return Client::Return::SEND;
}

Client::Return    Client::send(void) {
    size_t ret = ::send(this->_fd, (void*) this->_send_buffer.data(), this->_send_buffer.size(), MSG_DONTWAIT);
   if (ret <= 0) {
       return Client::Return::KICK;
   }
   if (ret == this->_send_buffer.size()) {
       this->_send_buffer.clear();
       return Client::Return::OK;
   }
   this->_send_buffer.erase(0, ret);
   return Client::Return::SEND;
}

bool    Client::decrypt(void) {
    std::string tag;
    int outlen, rv;
    size_t gcm_ivlen = 16;
    unsigned char outbuf[PIPE_BUF + 128 + 16 + 1];
    OSSL_PARAM params[2] = {
        OSSL_PARAM_END, OSSL_PARAM_END
    };

    //Get tag
    if (this->_packetsize < 16) {
        return false;
    }
    tag.assign(this->_encrypted_buffer.begin() + this->_packetsize - 16, this->_encrypted_buffer.begin() + this->_packetsize);

    //Put data to decrypt in buffer

    params[0] = OSSL_PARAM_construct_size_t(OSSL_CIPHER_PARAM_AEAD_IVLEN, &gcm_ivlen);

    /*
     * Initialise an encrypt operation with the cipher/mode, key, IV and
     * IV length parameter.
     */
    if (!EVP_DecryptInit_ex2(_ctx, _cipher, _key, _iv, params)) {
        return false;
    }

    /* Decrypt plaintext */
    if (!EVP_DecryptUpdate(_ctx, outbuf, &outlen, reinterpret_cast<unsigned char*>(this->_encrypted_buffer.data()), this->_packetsize - 16)) {
        return false;
    }

    /* Set expected tag value. */
    params[0] = OSSL_PARAM_construct_octet_string(OSSL_CIPHER_PARAM_AEAD_TAG, (void*)tag.data(), 16);

    if (!EVP_CIPHER_CTX_set_params(_ctx, params)) {
        return false;
    }

    if (outlen < 16) {
        return false;
    }
    memcpy(this->_iv, outbuf + outlen - 16, 16);
    this->_buffer.insert(this->_buffer.end(), outbuf, outbuf + outlen - 16);
    this->_encrypted_buffer.erase(this->_encrypted_buffer.begin(), this->_encrypted_buffer.begin() + this->_packetsize);
    /* Finalise: note get no output for GCM */
    rv = EVP_DecryptFinal_ex(_ctx, outbuf, &outlen);

    /*
     * Print out return value. If this is not successful authentication
     * failed and plaintext is not trustworthy.
     */
    if (rv <= 0) {
        return false;
    }
    //EVP_CIPHER_CTX_reset(this->_ctx);
    return true;
}

void    Client::increaseIV(void) {
    for (int i = 15; i >= 0; --i) {
        this->_iv[i] += 1;
        if (this->_iv[i] != 0)
            break;
    }
}
