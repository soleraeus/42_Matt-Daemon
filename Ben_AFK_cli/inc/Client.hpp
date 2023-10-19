/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/19 20:50:15 by bdetune           #+#    #+#             */
/*   Updated: 2023/10/19 22:24:59 by bdetune          ###   ########.fr       */
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

	private:
		bool				_secure;
		int					_sockfd;
		EVP_PKEY*			_RSA_key;
		BIO*				_mem;
		char*				_pubkey;
		long				_pubkey_len;
		struct sockaddr_in	_sockaddr;
};

#endif
