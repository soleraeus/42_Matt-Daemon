/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/19 20:50:15 by bdetune           #+#    #+#             */
/*   Updated: 2023/10/23 22:03:57 by bdetune          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MATT_DAEMON_BEN_AFK_CLI_CLIENT_HPP
# define MATT_DAEMON_BEN_AFK_CLI_CLIENT_HPP

# include <arpa/inet.h>
# include <iostream>
# include <sys/socket.h>
# include <unistd.h>
# include <string.h>
# include <system_error>
# include <sys/epoll.h>
# include <sys/types.h>

//Openssl
# include <openssl/evp.h>
# include <openssl/pem.h>
# include <openssl/rsa.h>

class Client {
	public:
		explicit Client(const char* ip = NULL, bool secure = false);
		explicit Client(const Client& src);
		explicit Client(Client&& src);

		~Client(void);

		Client&	operator=(const Client& rhs);
		Client&	operator=(Client&& rhs);

		void	printPubkey(void);
		int		run(void);

	private:
		bool				_secure;
		bool				_handshake;
		int					_sockfd;
		EVP_PKEY*			_RSA_key;
		BIO*				_mem;
		char*				_pubkey;
		long				_pubkey_len;
		struct sockaddr_in	_sockaddr;
		int					_epollfd;
		struct epoll_event	_event;
		std::string			_buf;
		size_t				_packetsize;
		unsigned char		_recv_buffer[PIPE_BUF + 14];
		ssize_t				_recv_len;
		unsigned char		_key[32];
		unsigned char		_iv[16];

		bool				getPacketSize(void);
};

#endif
