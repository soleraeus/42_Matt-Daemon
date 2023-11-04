/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/27 20:33:42 by bdetune           #+#    #+#             */
/*   Updated: 2023/11/04 13:36:41 by bdetune          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MATT_DAEMON_SERVER_HPP
# define MATT_DAEMON_SERVER_HPP

//std
# include <cstring>
# include <map>
# include <memory>
# include <system_error>

//C
# include <unistd.h>

//Unix
# include <arpa/inet.h>
# include <netinet/in.h>
# include <sys/epoll.h>
# include <sys/socket.h>
# include <sys/types.h>

//Openssl
# include <openssl/rand.h>

//Project specific
# include "Client.hpp"
# include "Matt_daemon.hpp"
# include "Tintin_reporter.hpp"


class Server
{
	public:
		enum class ServerType{
			SECURE_ONLY,
			SECURE,
			STANDARD
		};

		//Constructors
		Server(void);
		Server(std::shared_ptr<Tintin_reporter>& reporter) noexcept;
		Server(std::shared_ptr<Tintin_reporter>&& reporter) noexcept;
		Server(Server const & src);
		Server(Server && src);

		//Destructor
		~Server(void);

		//Operator overload
		Server&	operator=(Server const & rhs);
		Server&	operator=(Server && rhs);

		//Member functions
		bool	create_server(Server::ServerType type, const std::string& username, const std::string& password);
		void	serve(void);
	
	private:
        std::shared_ptr<int>				_sockfd;
        std::shared_ptr<int>    			_securesockfd;
        std::shared_ptr<int>				_epollfd;
		unsigned char						_key[32];
		std::shared_ptr<Tintin_reporter>	_reporter;
		struct epoll_event					_events[5];
		struct epoll_event					_init;
		std::map<int, Client>				_clients;
        std::string                         _username;
        std::string                         _password;

		//Private member functions
        bool    createStandardServer(void);
        bool    createSecureServer(void);
		bool	epoll_add(int fd, uint32_t events);
		bool	epoll_del(int fd);
		bool	epoll_mod(int fd, uint32_t events);
};

#endif
