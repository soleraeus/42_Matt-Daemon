/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/28 20:17:35 by bdetune           #+#    #+#             */
/*   Updated: 2023/10/26 21:11:29 by bdetune          ###   ########.fr       */
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
# include <iomanip>
# include <sys/types.h>
# include <sys/socket.h>
# include <unistd.h>
# include <memory>
# include <vector>
# include <openssl/bio.h>
# include <openssl/core_names.h>
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

        int                 _fd;
        int                 _packetsize;
        bool                _secure;
        bool                _handshake;
        unsigned char       _RSA_buf[1024];
        unsigned char*      _key;
        unsigned char       _iv[16];
        std::string         _username;
        std::string         _password;
        int                 _auth_tries; 
        bool                _authenticated;
        char                _recv_buffer[PIPE_BUF + 1];
        std::string         _buffer;
        std::string         _send_buffer;
        std::vector<char>   _encrypted_buffer;
        EVP_CIPHER_CTX*     _ctx;
        EVP_CIPHER*         _cipher;

        Client::Return      getPacketSize(void);
        Client::Return      flush(std::shared_ptr<Tintin_reporter>& reporter);
        Client::Return      sendKey(void);
        bool                decrypt(void);
        bool                encrypt(std::shared_ptr<Tintin_reporter>& reporter);
        void                increaseIV(void);
};

#endif
