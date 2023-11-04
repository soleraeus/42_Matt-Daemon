/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/28 20:17:35 by bdetune           #+#    #+#             */
/*   Updated: 2023/11/04 11:01:09 by bdetune          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MATT_DAEMON_CLIENT_HPP
# define MATT_DAEMON_CLIENT_HPP

//std
# include <climits>
# include <cstdlib>
# include <cstring>
# include <functional>
# include <iomanip>
# include <memory>
# include <string>
# include <system_error>
# include <vector>

//C
# include <stddef.h>
# include <unistd.h>

//Unix
# include <sys/socket.h>
# include <sys/types.h>

//Openssl
# include <openssl/bio.h>
# include <openssl/core_names.h>
# include <openssl/evp.h>
# include <openssl/pem.h>
# include <openssl/rand.h>
# include <openssl/rsa.h>

//Project specific
# include "Matt_daemon.hpp"
# include "Tintin_reporter.hpp"

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
        Client(int fd);
        Client(int fd, unsigned char* key, const std::string& username, const std::string& password);

        Client(const Client & src);
        Client(Client && src);

        ~Client(void);
        
        Client &    operator=(const Client & rhs);
        Client &    operator=(Client && rhs);

        [[nodiscard]] Client::Return    receive(std::shared_ptr<Tintin_reporter>& reporter);
        [[nodiscard]] Client::Return    send(void);

    private:

        std::shared_ptr<int>			_fd;
        int								_packetsize;
        bool							_secure;
        bool							_handshake;
        unsigned char					_RSA_buf[1024];
        unsigned char*					_key;
        unsigned char					_iv[16];
        std::string						_username;
        std::string						_password;
        int								_auth_tries; 
        bool							_authenticated;
        char							_recv_buffer[PIPE_BUF + 1];
        std::string						_buffer;
        std::string						_send_buffer;
        std::vector<char>				_encrypted_buffer;
        std::shared_ptr<EVP_CIPHER_CTX>	_ctx;
        EVP_CIPHER*						_cipher;

        Client::Return					getPacketSize(void);
        Client::Return					flush(std::shared_ptr<Tintin_reporter>& reporter);
        Client::Return					handshake(void);
        Client::Return					sendKey(void);
        bool							decrypt(void);
        bool							encrypt(std::shared_ptr<Tintin_reporter>& reporter);
        Client::Return					authenticate(std::shared_ptr<Tintin_reporter>& reporter);
        void							addHeader(void);
        void                            trim(std::string& msg);
        bool                            isValidUTF8(std::string& msg);
};

#endif
