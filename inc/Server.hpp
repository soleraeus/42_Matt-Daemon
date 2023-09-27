/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/27 20:33:42 by bdetune           #+#    #+#             */
/*   Updated: 2023/09/27 21:35:57 by bdetune          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP
# include "Matt_daemon.hpp"
# include "Tintin_reporter.hpp"
# include <sys/epoll.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <unistd.h>
# include <cstring>
# include <netinet/in.h>
# include <arpa/inet.h>

class Server
{
	public:
		//Constructors
		Server(void);
		Server(Tintin_reporter & reporter);
		Server(Server const & src);
		Server(Server && src);

		//Destructor
		~Server(void);

		//Operator overload
		Server&	operator=(Server const & rhs);
		Server&	operator=(Server && rhs);

		//Member functions
		bool	create_server(void);
		//void	serve(void);
	
	private:
		int					_sockfd;
		int					_epollfd;
		Tintin_reporter*	_reporter;
};

#endif
