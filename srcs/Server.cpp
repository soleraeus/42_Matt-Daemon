/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/27 20:42:18 by bdetune           #+#    #+#             */
/*   Updated: 2023/10/12 21:06:34 by bdetune          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(void): _sockfd(0), _securesockfd(0), _epollfd(0), _reporter(nullptr) {}
Server::Server(std::shared_ptr<Tintin_reporter>& reporter) noexcept: _sockfd(0), _securesockfd(0), _epollfd(0), _reporter(reporter) {}
Server::Server(std::shared_ptr<Tintin_reporter>&& reporter) noexcept: _sockfd(0), _securesockfd(0), _epollfd(0), _reporter(std::move(reporter)) {}

Server::Server(Server const & src): _sockfd(0), _securesockfd(0), _epollfd(0), _reporter(src._reporter), _clients(src._clients){
	memcpy(this->_key, src._key, 32);
	if (src._sockfd > 0)
		this->_sockfd = dup(src._sockfd);
	if (src._securesockfd > 0)
		this->_securesockfd = dup(src._securesockfd);
	if (src._epollfd > 0)
		this->_epollfd = dup(src._epollfd);
	if (this->_sockfd == -1 || this->_securesockfd == -1 || this->_epollfd == -1)
	{
		if (this->_sockfd > 0)
			close(this->_sockfd);
		if (this->_securesockfd > 0)
			close(this->_securesockfd);
		if (this->_epollfd > 0)
			close(this->_epollfd);
		throw std::system_error(std::error_code(), "Could not duplicate fds");
	}
}

Server::Server(Server && src): _sockfd(std::move(src._sockfd)), _securesockfd(std::move(src._securesockfd)), _epollfd(std::move(src._epollfd)), _reporter(std::move(src._reporter)), _clients(std::move(src._clients)){
	memcpy(this->_key, src._key, 32);
	src._sockfd = 0;
	src._securesockfd = 0;
	src._epollfd = 0;
}

Server::~Server(void)
{
	if (this->_epollfd > 0)
		close(this->_epollfd);
	if (this->_sockfd > 0)
		close(this->_sockfd);
	if (this->_securesockfd > 0)
		close(this->_securesockfd);
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
	if (this->_securesockfd)
		close(this->_securesockfd);

	memcpy(this->_key, rhs._key, 32);
	this->_sockfd = rhs._sockfd ? dup(rhs._sockfd) : 0;
	this->_securesockfd = rhs._securesockfd ? dup(rhs._securesockfd) : 0;
	this->_epollfd = rhs._epollfd ? dup(rhs._epollfd) : 0;
	this->_reporter = rhs._reporter;
	this->_clients = rhs._clients;
	return (*this);
}

Server & Server::operator=(Server && rhs)
{
	if (this == &rhs)
		return (*this);
	if (this->_epollfd)
		close(this->_epollfd);
	if (this->_sockfd)
		close(this->_sockfd);
	if (this->_securesockfd)
		close(this->_securesockfd);

	memcpy(this->_key, rhs._key, 32);
	this->_sockfd = std::move(rhs._sockfd);
	this->_securesockfd = std::move(rhs._securesockfd);
	this->_epollfd = std::move(rhs._epollfd);
	this->_reporter = std::move(rhs._reporter);
	this->_clients = std::move(rhs._clients);
	rhs._sockfd = 0;
	rhs._securesockfd = 0;
	rhs._epollfd = 0;
	return (*this);
}

bool	Server::create_server(Server::ServerType type)
{
	int					opt = 1;
	struct sockaddr_in	address;
	switch (type)
	{
		case Server::ServerType::STANDARD : 
			//Create standard server
			this->_sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
			if (this->_sockfd <= 0)
			{
				std::cerr << "Could not create socket" << std::endl;
				return (false);
			}
			memset(&address, 0, sizeof(address));
			address.sin_family = AF_INET;
			address.sin_addr.s_addr = INADDR_ANY;
			address.sin_port = htons(MATT_DAEMON_BASIC_PORT);
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
			break ;

		case Server::ServerType::SECURE : 
			//Create standard server
			this->_sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
			if (this->_sockfd <= 0)
			{
				std::cerr << "Could not create socket" << std::endl;
				return (false);
			}
			memset(&address, 0, sizeof(address));
			address.sin_family = AF_INET;
			address.sin_addr.s_addr = INADDR_ANY;
			address.sin_port = htons(MATT_DAEMON_BASIC_PORT);
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
				std::cerr << "Could not listen on " << MATT_DAEMON_BASIC_PORT << std::endl;
				return (false);
			}

			//Create secure server
			if (!RAND_bytes(this->_key, 32))
			{
				std::cerr << "Could not create key for secure server" << std::endl;
				return (false);
			}
			this->_securesockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
			if (this->_securesockfd <= 0)
			{
				std::cerr << "Could not create socket" << std::endl;
				return (false);
			}
			memset(&address, 0, sizeof(address));
			address.sin_family = AF_INET;
			address.sin_addr.s_addr = INADDR_ANY;
			address.sin_port = htons(MATT_DAEMON_SECURE_PORT);
			if (setsockopt(this->_securesockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(int)))
			{
				std::cerr << "Could not set option on socket" << std::endl;
				return (false);
			}
			if (bind(this->_securesockfd, (struct sockaddr *)&address, sizeof(address)) < 0)
			{
				std::cerr << "Could not bind socket" << std::endl;
				return (false);
			}
			if (listen(this->_securesockfd, 3) < 0)
			{
				std::cerr << "Could not listen on " << MATT_DAEMON_SECURE_PORT << std::endl;
				return (false);
			}
			break ;
		case Server::ServerType::SECURE_ONLY : 
			//Create secure server
			if (!RAND_bytes(this->_key, 32))
			{
				std::cerr << "Could not create key for secure server" << std::endl;
				return (false);
			}
			this->_securesockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
			if (this->_securesockfd <= 0)
			{
				std::cerr << "Could not create socket" << std::endl;
				return (false);
			}
			memset(&address, 0, sizeof(address));
			address.sin_family = AF_INET;
			address.sin_addr.s_addr = INADDR_ANY;
			address.sin_port = htons(MATT_DAEMON_SECURE_PORT);
			if (setsockopt(this->_securesockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(int)))
			{
				std::cerr << "Could not set option on socket" << std::endl;
				return (false);
			}
			if (bind(this->_securesockfd, (struct sockaddr *)&address, sizeof(address)) < 0)
			{
				std::cerr << "Could not bind socket" << std::endl;
				return (false);
			}
			if (listen(this->_securesockfd, 3) < 0)
			{
				std::cerr << "Could not listen on " << MATT_DAEMON_SECURE_PORT << std::endl;
				return (false);
			}
			break ;
		default:
			std::cerr << "Unknown value provided to create server" << std::endl;
			return (false);
	}
	this->_epollfd = epoll_create(1);
	if (this->_epollfd <= 0)
		return (false);
	return (true);
}

bool	Server::epoll_add(int fd, uint32_t events)
{
	memset(&this->_init, 0, sizeof(struct epoll_event));
	this->_init.data.fd = fd;
	this->_init.events = events;
	return (epoll_ctl(this->_epollfd, EPOLL_CTL_ADD, fd, &this->_init) != -1);
}

bool	Server::epoll_del(int fd)
{
	memset(&this->_init, 0, sizeof(struct epoll_event));
	this->_init.data.fd = fd;
	return (epoll_ctl(this->_epollfd, EPOLL_CTL_DEL, fd, &this->_init) != -1);
}

void	Server::serve(void)
{
	int								new_connection = 0;
	int								nb_events = 0;
	std::map<int, Client>::iterator	it;

	if (!this->_reporter)
		throw std::logic_error("No reporter associated with server");
	if (this->_epollfd <= 0 || (this->_sockfd <= 0 && this->_securesockfd <= 0))
		throw std::logic_error("Member function create_server needs to be called before serving");
	memset(this->_events, 0, sizeof(struct epoll_event) * 5);
	if (this->_sockfd && !this->epoll_add(this->_sockfd, EPOLLIN))
	{
		if (this->_reporter->log("Could not initialize epoll on socket", ERROR) != Tintin_reporter::Return::OK) {}
		return ;
	}
	if (this->_securesockfd && !this->epoll_add(this->_securesockfd, EPOLLIN))
	{
		if (this->_reporter->log("Could not initialize epoll on socket", ERROR) != Tintin_reporter::Return::OK) {}
		return ;
	}

	while (true)
	{
		nb_events = epoll_wait(this->_epollfd, this->_events, 5, 1000 * 60 * 15);
		if (g_sig > 0)
		{
			if (this->_reporter->log("Signal handler.", INFO) != Tintin_reporter::Return::OK) {}
			return ;
		}
		if (nb_events > 0)
		{
			for (int i = 0; i < nb_events; ++i)
			{
				[[likely]] if ((it = this->_clients.find(this->_events[i].data.fd)) != this->_clients.end())
				{
					switch (it->second.receive(this->_reporter))
					{
						case Client::Return::QUIT:
							if (this->_reporter->log("Request quit.", INFO) != Tintin_reporter::Return::OK) {}
							return ;
							break ;
						case Client::Return::KICK:
							this->epoll_del(this->_events[i].data.fd);
							this->_clients.erase(it);
							break ;
						[[likely]] default:
							break;
					}
				}
				else if (this->_events[i].data.fd == this->_sockfd)
				{
					new_connection = accept(this->_sockfd, NULL, NULL);
					if (new_connection <= 0)
					{
						if (this->_reporter->log("Could not accept new client", ERROR) != Tintin_reporter::Return::OK)
							return ;
					}
					if (this->_clients.size() == 3)
					{
						close(new_connection);
						if (this->_reporter->log("Connection attempt while maximum capacity of 3 connected clients has already been reached", ERROR) != Tintin_reporter::Return::OK)
							return ;
						continue ;
					}
					this->_clients[new_connection] = Client(new_connection);
					this->epoll_add(new_connection, EPOLLIN);
				}
				else if (this->_events[i].data.fd == this->_securesockfd)
				{
					new_connection = accept(this->_securesockfd, NULL, NULL);
					if (new_connection <= 0)
					{
						if (this->_reporter->log("Could not accept new client on secure port", ERROR) != Tintin_reporter::Return::OK)
							return ;
					}
					if (this->_clients.size() == 3)
					{
						close(new_connection);
						if (this->_reporter->log("Connection attempt while maximum capacity of 3 connected clients has already been reached", ERROR) != Tintin_reporter::Return::OK)
							return ;
						continue ;
					}
					try 
					{
						this->_clients.insert(std::pair<int, Client>(new_connection, Client(new_connection, true, this->_key)));
						this->epoll_add(new_connection, EPOLLIN);
					}
					catch (std::system_error const & e)
					{
						if (this->_reporter->log("Could not generate iv for Client", ERROR) != Tintin_reporter::Return::OK)
							return ;
					}
				}
			}
		}
	}
}
