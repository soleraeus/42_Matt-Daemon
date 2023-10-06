/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/28 20:21:03 by bdetune           #+#    #+#             */
/*   Updated: 2023/10/06 20:53:42 by bdetune          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(void): _fd(0){}
Client::Client(int fd): _fd(fd){}

Client::Client(const Client & src): _fd(dup(src._fd)), _buffer(src._buffer) {}
Client::Client(Client && src): _fd(std::move(src._fd)), _buffer(std::move(src._buffer)) {
	src._fd = 0;
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
	this->_buffer = rhs._buffer;
	return (*this);
}

Client & Client::operator=(Client && rhs)
{
	if (this == &rhs)
		return (*this);
	this->_fd = rhs._fd;
	rhs._fd = 0;
	this->_buffer = std::move(rhs._buffer);
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
