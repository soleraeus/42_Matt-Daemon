/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/27 20:42:18 by bdetune           #+#    #+#             */
/*   Updated: 2023/09/27 21:37:06 by bdetune          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(void): _sockfd(0), _epollfd(0), _reporter(NULL) {}
Server::Server(Tintin_reporter & reporter): _sockfd(0), _epollfd(0), _reporter(&reporter) {}

Server::Server(Server const & src): _sockfd(src._sockfd), _epollfd(src._epollfd), _reporter(src._reporter){}
Server::Server(Server && src): _sockfd(std::move(_sockfd)), _epollfd(std::move(src._epollfd)), _reporter(std::move(src._reporter)){
	src._sockfd = 0;
	src._epollfd = 0;
	src._reporter = NULL;
}

Server::~Server(void)
{
	if (this->_epollfd > 0)
		close(this->_epollfd);
	if (this->_sockfd > 0)
		close(this->_sockfd);
	return ;
}

Server & Server::operator=(Server const & rhs)
{
	if (this == &rhs)
		return (*this);
	if (this->_epollfd)
		close(this->_epollfd);
	if (this->_sockfd)
		close(this->_sockfd);
	this->_sockfd = rhs._sockfd;
	this->_epollfd = rhs._epollfd;
	this->_reporter = rhs._reporter;
	return (*this);
}

Server & Server::operator=(Server && rhs)
{
	if (this->_epollfd)
		close(this->_epollfd);
	if (this->_sockfd)
		close(this->_sockfd);
	this->_sockfd = std::move(rhs._sockfd);
	this->_epollfd = std::move(rhs._epollfd);
	this->_reporter = std::move(rhs._reporter);
	rhs._sockfd = 0;
	rhs._epollfd = 0;
	rhs._reporter = NULL;
	return (*this);
}

bool	Server::create_server(void)
{
	int					opt = 1;
	struct sockaddr_in	address;

	this->_sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
	if (this->_sockfd <= 0)
	{
		std::cerr << "Could not create socket" << std::endl;
		return (false);
	}
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(4242);
	if (setsockopt(this->_sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(int)))
	{
		std::cerr << "Could not set option on socket" << std::endl;
		return (false);
	}
	if (bind(this->_sockfd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		std::cerr << "Could not bind socket" << std::endl;
		return (false);
	}
	if (listen(this->_sockfd, 3) < 0)
	{
		std::cerr << "Could not listen on 4242" << std::endl;
		return (false);
	}
	this->_epollfd = epoll_create(1);
	if (this->_epollfd <= 0)
		return (false);
	return (true);
}


