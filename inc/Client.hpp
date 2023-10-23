/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/28 20:17:35 by bdetune           #+#    #+#             */
/*   Updated: 2023/10/23 19:59:01 by bdetune          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP
# include "Matt_daemon.hpp"
# include "Tintin_reporter.hpp"
# include <string>
# include <climits>
# include <stddef.h>
# include <cstdlib>
# include <cstring>
# include <sys/types.h>
# include <sys/socket.h>
# include <unistd.h>
# include <memory>
# include <vector>
# include <openssl/bio.h>
# include <openssl/rand.h>
# include <openssl/evp.h>
# include <openssl/pem.h>
# include <openssl/rsa.h>
# include <system_error>

class Client
{
	public:
		enum class Return {
			QUIT,
			KICK,
			SEND,
			OK
		};

		Client(void);
		Client(int fd, bool secure = false, unsigned char* key = NULL);

		Client(const Client & src);
		Client(Client && src);

		~Client(void);
		
		Client &	operator=(const Client & rhs);
		Client &	operator=(Client && rhs);

		[[nodiscard]] Client::Return	receive(std::shared_ptr<Tintin_reporter>& reporter);

	private:

		int					_fd;
		int					_packetsize;
		bool				_secure;
		bool				_handshake;
		unsigned char		_RSA_buf[1024];
		unsigned char*		_key;
		unsigned char		_iv[16];
		char				_recv_buffer[PIPE_BUF + 1];
		std::string			_buffer;
		std::vector<char>	_encrypted_buffer;

		Client::Return		getPacketSize(void);
		Client::Return		flush(std::shared_ptr<Tintin_reporter>& reporter);
		Client::Return		sendKey(void);
};

#endif
