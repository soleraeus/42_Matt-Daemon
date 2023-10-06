/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/28 20:17:35 by bdetune           #+#    #+#             */
/*   Updated: 2023/10/06 20:30:58 by bdetune          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP
# include "Matt_daemon.hpp"
# include "Tintin_reporter.hpp"
# include <string>
# include <climits>
# include <stddef.h>
# include <cstring>
# include <sys/types.h>
# include <sys/socket.h>
# include <unistd.h>
# include <memory>

class Client
{
	public:
		Client(void);
		Client(int fd);

		Client(const Client & src);
		Client(Client && src);

		~Client(void);
		
		Client &	operator=(const Client & rhs);
		Client &	operator=(Client && rhs);

		bool		receive(std::shared_ptr<Tintin_reporter> reporter);

	private:

		int				_fd;
		char			_recv_buffer[PIPE_BUF + 1];
		std::string		_buffer;
};

#endif
