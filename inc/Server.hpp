/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/27 20:33:42 by bdetune           #+#    #+#             */
/*   Updated: 2023/10/12 20:27:21 by bdetune          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP
# include "Matt_daemon.hpp"
# include "Tintin_reporter.hpp"
# include "Client.hpp"
# include <sys/epoll.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <unistd.h>
# include <cstring>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <map>
# include <memory>
# include <system_error>
# include <openssl/rand.h>


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
		bool	create_server(Server::ServerType type);
		void	serve(void);
	
	private:
		int									_sockfd;
		int									_securesockfd;
		int									_epollfd;
		unsigned char						_key[32];
		std::shared_ptr<Tintin_reporter>	_reporter;
		struct epoll_event					_events[5];
		struct epoll_event					_init;
		std::map<int, Client>				_clients;

		//Private member functions
		bool	epoll_add(int fd, uint32_t events);
		bool	epoll_del(int fd);
};

#endif
