/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/28 20:21:03 by bdetune           #+#    #+#             */
/*   Updated: 2023/10/12 21:01:25 by bdetune          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(void): _fd(0), _secure(false), _key(NULL){}
Client::Client(int fd, bool secure, unsigned char* key): _fd(fd), _secure(secure), _key(key){
	if (secure)
	{
		if (!RAND_bytes(this->_iv, 16))
		{
			throw std::system_error(std::error_code(), "Could not generate iv");
		}
	}	
}

Client::Client(const Client & src): _fd(dup(src._fd)), _secure(src._secure), _key(src._key), _buffer(src._buffer) {
	memcpy(this->_iv, src._iv, 16);
}
Client::Client(Client && src): _fd(std::move(src._fd)), _secure(src._secure), _key(src._key), _buffer(std::move(src._buffer)) {
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
	this->_secure = rhs._secure;
	this->_buffer = rhs._buffer;
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
	this->_secure = rhs._secure;
	this->_buffer = std::move(rhs._buffer);
	this->_key = rhs._key;
	rhs._key = NULL;
	memcpy(this->_iv, rhs._iv, 16);

	return (*this);
}

Client::Return	Client::receive(std::shared_ptr<Tintin_reporter> reporter)
{
	ssize_t					len = 0;
	std::string::size_type	pos;
	std::string				sub;
	
	len = recv(this->_fd, (void *)this->_recv_buffer, PIPE_BUF, MSG_DONTWAIT);
	if (len <= 0)
		return Client::Return::KICK;
	this->_recv_buffer[len] ='\0';
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
