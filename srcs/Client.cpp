/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/28 20:21:03 by bdetune           #+#    #+#             */
/*   Updated: 2023/10/13 21:32:23 by bdetune          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(void): _fd(0), _packetsize(0), _secure(false), _key(NULL){}
Client::Client(int fd, bool secure, unsigned char* key): _fd(fd), _packetsize(0), _secure(secure), _key(key){
	if (secure)
	{
		if (!RAND_bytes(this->_iv, 16))
		{
			throw std::system_error(std::error_code(), "Could not generate iv");
		}
	}	
}

Client::Client(const Client & src): _fd(dup(src._fd)), _packetsize(src._packetsize), _secure(src._secure), _key(src._key), _buffer(src._buffer), _header(src._header) {
	memcpy(this->_iv, src._iv, 16);
}
Client::Client(Client && src): _fd(std::move(src._fd)), _packetsize(src._packetsize), _secure(src._secure), _key(src._key), _buffer(std::move(src._buffer)), _header(std::move(src._header)) {
	memcpy(this->_iv, src._iv, 16);
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
	this->_buffer = rhs._buffer;
	this->_header = rhs._header;
	this->_key = rhs._key;
	memcpy(this->_iv, rhs._iv, 16);
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
	this->_buffer = std::move(rhs._buffer);
	this->_header = std::move(rhs._header);
	this->_key = rhs._key;
	rhs._key = NULL;
	memcpy(this->_iv, rhs._iv, 16);

	return (*this);
}

Client::Return	Client::getPacketSize(ssize_t& size)
{
	size_t	prev = this->_header.size();
	size_t	pos = 0;

	this->_header.insert(this->_header.end(), this->_encrypted_buffer.begin(), this->_encrypted_buffer.end());
	for (size_t i = 0; i < this->_header.size(); ++i)
	{
		std::cerr << "Found \\n at position" << i << std::endl;
		if (this->_header[i] == '\n')
		{
			if (i < 8 || i > 12)
				return Client::Return::KICK;
			if (memcmp((void *)"Length ",(void *)&this->_header[0], 7))
				return Client::Return::KICK;
			pos = i;
			if (this->_header[i - 1] == '\r')
				pos = i - 1;
			if (pos == 7)
				return Client::Return::KICK;
			this->_header[pos] = '\0';
			for (size_t j = 7; j < pos; ++j)
			{
				if (this->_header[j] < '0' || this->_header[j] > '9')
					return Client::Return::KICK;
			}
			this->_packetsize = std::atoi(&this->_header[7]);
			if (this->_packetsize > PIPE_BUF)
				return Client::Return::KICK;
			size = i - prev;
			return Client::Return::OK;
		}
		if (this->_header[i] < ' ' || this->_header[i] > 'z') {
			return Client::Return::KICK;
		}
	}
	size = -1;
	if (this->_header.size() > 12)
		return Client::Return::KICK;
	return Client::Return::OK;
}

Client::Return	Client::receive(std::shared_ptr<Tintin_reporter> reporter)
{
	ssize_t					rm;
	ssize_t					len = 0;
	std::string::size_type	pos;
	std::string				sub;
	Client::Return			ret;
	
	len = recv(this->_fd, (void *)this->_recv_buffer, PIPE_BUF, MSG_DONTWAIT);
	if (len <= 0)
		return Client::Return::KICK;
    std::cerr << "Received " << len << " bytes" << std::endl;
	this->_recv_buffer[len] ='\0';
	if (this->_secure)
	{
		this->_encrypted_buffer.insert(this->_encrypted_buffer.end(), this->_recv_buffer, &this->_recv_buffer[len]);
		if (!this->_packetsize)
		{
			ret = this->getPacketSize(rm);
			if (ret != Client::Return::OK)
				return ret;
			if (rm < 0)
				return Client::Return::OK;
			std::cerr << "To remove: " << rm << std::endl;
			this->_header.clear();
			this->_encrypted_buffer.erase(this->_encrypted_buffer.begin(), this->_encrypted_buffer.begin() + rm + 1);
			rm = 0;
		}
		if (this->_packetsize > 0 && this->_encrypted_buffer.size() >= static_cast<size_t>(this->_packetsize))
		{
			std::cerr << "It would be time to decrypt" << std::endl;
			this->_buffer += std::string(this->_encrypted_buffer.begin(), this->_encrypted_buffer.begin() + this->_packetsize);
			this->_encrypted_buffer.erase(this->_encrypted_buffer.begin(), this->_encrypted_buffer.begin() + this->_packetsize);
			this->_packetsize = 0;
		}
	}
	else
		this->_buffer += this->_recv_buffer;
	while ((pos = this->_buffer.find('\r')) != std::string::npos)
	{
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
	return Client::Return::OK;
}
