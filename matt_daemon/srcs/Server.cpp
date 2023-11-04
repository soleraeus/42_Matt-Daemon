/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/27 20:42:18 by bdetune           #+#    #+#             */
/*   Updated: 2023/11/04 13:38:59 by bdetune          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

static const std::function<void(int*)>	closeOnDelete = [](int* ptr) -> void {
												                                if (ptr != nullptr) {
												                                    if (*ptr > 0) {
												                                        close(*ptr);
                                                                                    }
												                                    delete ptr;
												                                }
												                            };


Server::Server(void):
        _sockfd(nullptr, closeOnDelete),
        _securesockfd(nullptr, closeOnDelete),
        _epollfd(nullptr, closeOnDelete),
        _reporter(nullptr) {}

Server::Server(std::shared_ptr<Tintin_reporter>& reporter) noexcept:
        _sockfd(nullptr, closeOnDelete),
        _securesockfd(nullptr, closeOnDelete),
        _epollfd(nullptr, closeOnDelete),
        _reporter(reporter) {}

Server::Server(std::shared_ptr<Tintin_reporter>&& reporter) noexcept:
        _sockfd(nullptr, closeOnDelete),
        _securesockfd(nullptr, closeOnDelete),
        _epollfd(nullptr, closeOnDelete),
        _reporter(std::move(reporter)) {}

Server::Server(Server const & src):
        _sockfd(src._sockfd),
        _securesockfd(src._securesockfd),
        _epollfd(src._epollfd),
        _reporter(src._reporter),
        _clients(src._clients),
        _username(src._username),
        _password(src._password) {
    memcpy(this->_key, src._key, 32);
}

Server::Server(Server && src):
        _sockfd(std::move(src._sockfd)),
        _securesockfd(std::move(src._securesockfd)),
        _epollfd(std::move(src._epollfd)),
        _reporter(std::move(src._reporter)),
        _clients(std::move(src._clients)),
        _username(std::move(src._username)),
        _password(std::move(src._password)) {
    memcpy(this->_key, src._key, 32);
}

Server::~Server(void) {}

Server & Server::operator=(Server const & rhs)
{
    if (this == &rhs)
        return (*this);
    memcpy(this->_key, rhs._key, 32);
    this->_sockfd = rhs._sockfd;
    this->_securesockfd = rhs._securesockfd;
    this->_epollfd = rhs._epollfd;
    this->_reporter = rhs._reporter;
    this->_clients = rhs._clients;
    this->_username = rhs._username;
    this->_password = rhs._password;
    return (*this);
}

Server & Server::operator=(Server && rhs)
{
    if (this == &rhs)
        return (*this);
    memcpy(this->_key, rhs._key, 32);
    this->_sockfd = std::move(rhs._sockfd);
    this->_securesockfd = std::move(rhs._securesockfd);
    this->_epollfd = std::move(rhs._epollfd);
    this->_reporter = std::move(rhs._reporter);
    this->_clients = std::move(rhs._clients);
    this->_username = std::move(rhs._username);
    this->_password = std::move(rhs._password);
    return (*this);
}

bool    Server::createStandardServer(void) {
    int                 opt = 1;
    struct sockaddr_in  address;

    this->_sockfd = std::shared_ptr<int>(new int (socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0)), closeOnDelete);
    if (*(this->_sockfd) <= 0)
    {
        std::cerr << "Could not create socket" << std::endl;
        return false;
    }
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(MATT_DAEMON_BASIC_PORT);
    if (setsockopt(*(this->_sockfd), SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(int)))
    {
        std::cerr << "Could not set option on socket" << std::endl;
        return false;
    }
    if (bind(*(this->_sockfd), (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cerr << "Could not bind socket" << std::endl;
        return false;
    }
    if (listen(*(this->_sockfd), 3) < 0)
    {
        std::cerr << "Could not listen on 4242" << std::endl;
        return false;
    }
    return true;
}

bool    Server::createSecureServer(void) {
    int                 opt = 1;
    struct sockaddr_in  address;

    if (!RAND_bytes(this->_key, 32))
    {
        std::cerr << "Could not create key for secure server" << std::endl;
        return false;
    }
    this->_securesockfd = std::shared_ptr<int>(new int(socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0)), closeOnDelete);
    if (*(this->_securesockfd) <= 0)
    {
        std::cerr << "Could not create socket" << std::endl;
        return false;
    }
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(MATT_DAEMON_SECURE_PORT);
    if (setsockopt(*(this->_securesockfd), SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(int)))
    {
        std::cerr << "Could not set option on socket" << std::endl;
        return false;
    }
    if (bind(*(this->_securesockfd), (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cerr << "Could not bind socket" << std::endl;
        return false;
    }
    if (listen(*(this->_securesockfd), 3) < 0)
    {
        std::cerr << "Could not listen on " << MATT_DAEMON_SECURE_PORT << std::endl;
        return false;
    }
    return true;
}

bool    Server::create_server(Server::ServerType type, const std::string& username, const std::string& password)
{
    this->_username = username;
    this->_password = password;
    switch (type)
    {
        case Server::ServerType::STANDARD : 
            if (!this->createStandardServer())
                return false;
            break ;
        case Server::ServerType::SECURE : 
            if (!this->createStandardServer() || ! this->createSecureServer())
                return false;
            break ;
        case Server::ServerType::SECURE_ONLY : 
            if (!this->createSecureServer())
                return false;
            break ;
        default:
            std::cerr << "Unknown value provided to create server" << std::endl;
            return false;
    }
    this->_epollfd = std::shared_ptr<int>(new int(epoll_create(1)), closeOnDelete);
    if (*(this->_epollfd) <= 0) {
        std::cerr << "Could not create epollfd" << std::endl;
        return false;
    }
    return true;
}

bool    Server::epoll_add(int fd, uint32_t events)
{
    memset(&this->_init, 0, sizeof(struct epoll_event));
    this->_init.data.fd = fd;
    this->_init.events = events;
    return (epoll_ctl(*(this->_epollfd), EPOLL_CTL_ADD, fd, &this->_init) != -1);
}

bool    Server::epoll_del(int fd)
{
    memset(&this->_init, 0, sizeof(struct epoll_event));
    this->_init.data.fd = fd;
    return (epoll_ctl(*(this->_epollfd), EPOLL_CTL_DEL, fd, &this->_init) != -1);
}

bool    Server::epoll_mod(int fd, uint32_t events) {
    memset(&this->_init, 0, sizeof(struct epoll_event));
    this->_init.data.fd = fd;
    this->_init.events = events;
    return (epoll_ctl(*(this->_epollfd), EPOLL_CTL_MOD, fd, &this->_init) != -1);
}

void    Server::serve(void)
{
    int                             new_connection = 0;
    int                             nb_events = 0;
    std::map<int, Client>::iterator it;

    if (!this->_reporter)
        throw std::logic_error("No reporter associated with server");
    if (*(this->_epollfd) <= 0 || (*(this->_sockfd) <= 0 && *(this->_securesockfd) <= 0))
        throw std::logic_error("Member function create_server needs to be called before serving");
    memset(this->_events, 0, sizeof(struct epoll_event) * 5);
    if (*(this->_sockfd) > 0 && !this->epoll_add(*(this->_sockfd), EPOLLIN))
    {
        if (this->_reporter->log("Could not initialize epoll on socket", Tintin_reporter::Loglevel::ERROR) != Tintin_reporter::Return::OK) {}
        return ;
    }
    if (*(this->_securesockfd) > 0 && !this->epoll_add(*(this->_securesockfd), EPOLLIN))
    {
        if (this->_reporter->log("Could not initialize epoll on socket", Tintin_reporter::Loglevel::ERROR) != Tintin_reporter::Return::OK) {}
        return ;
    }

    while (true)
    {
        nb_events = epoll_wait(*(this->_epollfd), this->_events, 5, 1000 * 60 * 15);
        if (g_sig > 0)
        {
            if (this->_reporter->log("Signal handler.", Tintin_reporter::Loglevel::INFO) != Tintin_reporter::Return::OK) {}
            return ;
        }
        if (nb_events > 0)
        {
            for (int i = 0; i < nb_events; ++i)
            {
                [[likely]] if ((it = this->_clients.find(this->_events[i].data.fd)) != this->_clients.end())
                {
                    if (this->_events[i].events & EPOLLOUT) {
                        switch (it->second.send()) {
                            case Client::Return::KICK:
                                this->epoll_del(this->_events[i].data.fd);
                                this->_clients.erase(it);
                                break ;
                            case Client::Return::SEND:
                                break ;
                            [[likely]] default:
                                this->epoll_mod(this->_events[i].data.fd, EPOLLIN);
                                break;
                        }
                        continue ;
                    }
                    switch (it->second.receive(this->_reporter))
                    {
                        case Client::Return::QUIT:
                            if (this->_reporter->log("Request quit.", Tintin_reporter::Loglevel::INFO) != Tintin_reporter::Return::OK) {}
                            return ;
                            break ;
                        case Client::Return::KICK:
                            this->epoll_del(this->_events[i].data.fd);
                            this->_clients.erase(it);
                            break ;
                        case Client::Return::SEND:
                            this->epoll_mod(this->_events[i].data.fd, EPOLLOUT);
                            break ;
                        [[likely]] default:
                            break;
                    }
                }
                else if (this->_events[i].data.fd == *(this->_sockfd))
                {
                    new_connection = accept(*(this->_sockfd), NULL, NULL);
                    if (new_connection <= 0)
                    {
                        if (this->_reporter->log("Could not accept new client", Tintin_reporter::Loglevel::ERROR) != Tintin_reporter::Return::OK)
                            return ;
                    }
                    if (this->_clients.size() == 3)
                    {
                        close(new_connection);
                        if (this->_reporter->log("Connection attempt while maximum capacity of 3 connected clients has already been reached", Tintin_reporter::Loglevel::ERROR) != Tintin_reporter::Return::OK)
                            return ;
                        continue ;
                    }
                    this->_clients[new_connection] = Client(new_connection);
                    this->epoll_add(new_connection, EPOLLIN);
                }
                else if (this->_events[i].data.fd == *(this->_securesockfd))
                {
                    new_connection = accept(*(this->_securesockfd), NULL, NULL);
                    if (new_connection <= 0)
                    {
                        if (this->_reporter->log("Could not accept new client on secure port", Tintin_reporter::Loglevel::ERROR) != Tintin_reporter::Return::OK)
                            return ;
                    }
                    if (this->_clients.size() == 3)
                    {
                        close(new_connection);
                        if (this->_reporter->log("Connection attempt while maximum capacity of 3 connected clients has already been reached", Tintin_reporter::Loglevel::ERROR) != Tintin_reporter::Return::OK)
                            return ;
                        continue ;
                    }
                    try 
                    {
                        this->_clients.insert(std::pair<int, Client>(new_connection, Client(new_connection, this->_key, this->_username, this->_password)));
                        this->epoll_add(new_connection, EPOLLIN);
                    }
                    catch (std::system_error const & e)
                    {
                        if (this->_reporter->log("Could not generate iv for Client", Tintin_reporter::Loglevel::ERROR) != Tintin_reporter::Return::OK)
                            return ;
                    }
                }
            }
        }
    }
}
