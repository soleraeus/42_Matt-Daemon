/* ************************************************************************** */
/*                                                                                                                                                        */
/*                                                                                                                :::            ::::::::     */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                                                                        +:+ +:+                 +:+         */
/*     By: bdetune <marvin@42.fr>                                         +#+    +:+             +#+                */
/*                                                                                                +#+#+#+#+#+     +#+                     */
/*     Created: 2023/10/19 20:55:26 by bdetune                     #+#        #+#                         */
/*   Updated: 2023/11/06 19:35:04 by bdetune          ###   ########.fr       */
/*                                                                                                                                                        */
/* ************************************************************************** */

#include "Client.hpp"

static const std::function<void(EVP_PKEY*)> freeEVPPKey = [](EVP_PKEY* ptr) -> void {if (ptr != nullptr){EVP_PKEY_free(ptr);}};

static const std::function<void(BIO*)> freeBIO = [](BIO* ptr) -> void {if (ptr != nullptr){BIO_free(ptr);}};

static const std::function<void(EVP_CIPHER*)> freeEVPCipher = [](EVP_CIPHER* ptr) -> void {if (ptr != nullptr){EVP_CIPHER_free(ptr);}};

static const std::function<void(EVP_CIPHER_CTX*)> freeEVPCipherCtx = [](EVP_CIPHER_CTX* ptr) -> void {if (ptr != nullptr){EVP_CIPHER_CTX_free(ptr);}};

Client::Client(const char* ip, bool secure):
        _secure(secure),
        _handshake(false),
        _sockfd(0),
        _RSA_key(nullptr, freeEVPPKey),
        _mem(nullptr, freeBIO),
        _pubkey(nullptr),
        _pubkey_len(0),
        _epollfd(0),
        _ctx(nullptr, freeEVPCipherCtx),
        _cipher(nullptr, freeEVPCipher) {
    if (secure)
        this->initRSA();
    bzero(&_sockaddr, sizeof(_sockaddr));
    if ((_sockfd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0)) <= 0)
        throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not create socket");
    _sockaddr.sin_family = AF_INET;
    _sockaddr.sin_port = secure ? htons(4343) : htons(4242);
    if (inet_pton(AF_INET, ip ? ip : "127.0.0.1", &_sockaddr.sin_addr) <= 0) {
        close(_sockfd);
        throw std::system_error(std::make_error_code(std::errc::bad_address), "Bad ip address provided");
    }
}

Client::Client(const Client& src):
        _secure(src._secure),
        _handshake(src._handshake),
        _sockfd(0),
        _RSA_key(src._RSA_key),
        _mem(src._mem),
        _pubkey(src._pubkey),
        _pubkey_len(src._pubkey_len),
        _epollfd(0),
        _buf(src._buf),
        _inbuf(src._inbuf),
        _ctx(src._ctx),
        _cipher(src._cipher) {
    memcpy((void *)&_sockaddr, (void *)&src._sockaddr, sizeof(_sockaddr));
    memcpy((void *)_key, (void *)src._key, 32);
    memcpy((void *)_iv, (void *)src._iv, 16);
    if (src._sockfd > 0) {
        _sockfd = dup(src._sockfd);
        if (_sockfd <= 0)
             throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not duplicate socket");
    }
    if (src._epollfd > 0) {
        _epollfd = dup(src._epollfd);
        if (_epollfd <= 0) {
            if (_sockfd > 0)
                close(_sockfd);
            throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not duplicate epoll fd");
        }
    }
}

Client::Client(Client&& src):
        _secure(src._secure),
        _handshake(src._handshake),
        _sockfd(src._sockfd),
        _RSA_key(std::move(src._RSA_key)),
        _mem(std::move(src._mem)),
        _pubkey(src._pubkey),
        _pubkey_len(src._pubkey_len),
        _epollfd(src._epollfd),
        _buf(src._buf),
        _inbuf(src._inbuf),
        _ctx(std::move(src._ctx)),
        _cipher(std::move(src._cipher)) {
    memcpy((void *)&_sockaddr, (void *)&src._sockaddr, sizeof(_sockaddr));
    memcpy((void *)_key, (void *)src._key, 32);
    memcpy((void *)_iv, (void *)src._iv, 16);
    src._sockfd = 0;
    src._pubkey = nullptr;
    src._epollfd = 0;
}

Client::~Client(void) {
    if (_sockfd > 0)
        close(_sockfd);
    if (_epollfd > 0)
        close(_epollfd);
}

Client& Client::operator=(const Client& rhs) {
    if (_sockfd > 0) {
        close(_sockfd);
        _sockfd = 0;
    }
    if (_epollfd > 0) {
        close(_epollfd);
        _epollfd = 0;
    }
    if (this == &rhs)
        return (*this);
    _secure = rhs._secure;
    _handshake = rhs._handshake;
    _RSA_key = rhs._RSA_key;
    _mem = rhs._mem;
    _pubkey = rhs._pubkey;
    _pubkey_len = rhs._pubkey_len;
    _buf = rhs._buf;
    _inbuf = rhs._inbuf;
    _ctx = rhs._ctx;
    _cipher = rhs._cipher;
    memcpy((void *)&_sockaddr, (void *)&rhs._sockaddr, sizeof(_sockaddr));
    memcpy((void *)_key, (void *)rhs._key, 32);
    memcpy((void *)_iv, (void *)rhs._iv, 16);
    if (rhs._sockfd >= 0) {
        _sockfd = dup(rhs._sockfd);
        if (_sockfd <= 0)
            throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not duplicate socket");
    }
    if (rhs._epollfd > 0) {
        _epollfd = dup(rhs._epollfd);
        if (_epollfd <= 0) {
            if (_sockfd > 0)
                close(_sockfd);
            throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not duplicate epoll fd");
        }
    }
    return (*this);
}

Client& Client::operator=(Client&& rhs) {
    if (this == &rhs)
        return (*this);
    if (_sockfd > 0) {
        close(_sockfd);
        _sockfd = 0;
    }
    if (_epollfd > 0) {
        close(_epollfd);
        _epollfd = 0;
    }
    _secure = rhs._secure;
    _handshake = rhs._handshake;
    _sockfd = rhs._sockfd;
    rhs._sockfd = 0;
    _RSA_key = std::move(rhs._RSA_key);
    _mem = std::move(rhs._mem);
    _pubkey = rhs._pubkey;
    rhs._pubkey = nullptr;
    _pubkey_len = rhs._pubkey_len;
    memcpy((void *)&_sockaddr, (void *)&rhs._sockaddr, sizeof(_sockaddr));
    memcpy((void *)_key, (void *)rhs._key, 32);
    memcpy((void *)_iv, (void *)rhs._iv, 16);
    _epollfd = rhs._epollfd;
    rhs._epollfd = 0;
    _ctx = std::move(rhs._ctx);
    _cipher = std::move(rhs._cipher);
    return (*this);
}

bool    Client::getPacketSize(void)
{
    size_t    pos = 0;

    this->_packetsize = 0;
    for (size_t i = 0; i < this->_buf.size(); ++i)
    {
        if (this->_buf[i] == '\n')
        {
            if (i < 8 || i > 12
                || memcmp((void *)"Length ",(void *)&this->_buf[0], 7)) {
                return false;
            }
            pos = i;
            if (this->_buf[i - 1] == '\r')
                pos = i - 1;
            if (pos == 7)
                return false;
            this->_buf[pos] = '\0';
            for (size_t j = 7; j < pos; ++j)
            {
                if (this->_buf[j] < '0' || this->_buf[j] > '9')
                    return false;
            }
            this->_packetsize = std::atoi(&this->_buf[7]);
            if (this->_packetsize > PIPE_BUF + 128 + 16 + 16)
                return false;
            this->_buf.erase(this->_buf.begin(), this->_buf.begin() + i + 1);
            return true;
        }
        if ((this->_buf[i] < ' ' || this->_buf[i] > 'z')
            && this->_buf[i] != '\r') {
            return false;
        }
    }
    if (this->_buf.size() > 12)
        return false;
    return true;
}

bool    Client::getCredentials(void) {
    std::string username;
    std::string password;

    std::cout << "Please authenticate in order to use Matt Daemon" << std::endl;
    do {
        std::cout << "Username (between 5 and 128 characters): ";
        std::getline(std::cin, username);
        if (username.size() >= 5 && username.size() <= 128)
            break ;
        else
            std::cout << "Invalid username provided" << std::endl;
    } while (!std::cin.eof());
    if (std::cin.eof()) {
        std::cerr << std::endl << "No username provided, leaving Ben AFK" << std::endl;
        return false;
    }

    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    do {
        std::cout << "Password (between 12 and 128 characters): ";
        std::getline(std::cin, password);
        if (password.size() >= 12 && password.size() <= 128)
            break ;
        else
            std::cout << "Invalid password provided" << std::endl;

    } while (!std::cin.eof());
    if (std::cin.eof()) {
        std::cerr << std::endl << "No password provided, leaving Ben AFK" << std::endl;
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return false;
    }
    std::cout << std::endl;
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    this->_buf = username + "\n" + password + "\n";
    return true;
}

bool    Client::getUserInput(bool& quit, bool& logs) {
    std::cout << "Ben_AFK > ";
    std::getline(std::cin, _buf);
    _buf += "\n";
    quit = _buf == "quit\n";
    logs = _secure && _buf == "log?\n";
    if (!std::cin.eof())
        return true;
    std::cout << "Goodbye" << std::endl;
    epoll_ctl(_epollfd, EPOLL_CTL_DEL, _sockfd, &_event);
    return false;
}

bool    Client::handshake(void) {
    ssize_t     ret = 0;

    if (_event.events & EPOLLOUT) {
        if (_buf.empty()) {
            _buf = "Length " + std::to_string(_pubkey_len);
            _buf += "\n";
            _buf.insert(_buf.end(), _pubkey, _pubkey + _pubkey_len);
            ret = send(_sockfd, _buf.data(), _buf.size(), MSG_DONTWAIT);
            if (ret <= 0) {
                std::cerr << "Could not send packet to server" << std::endl;
                return false;
            }
            _buf.erase(_buf.begin(), _buf.begin() + ret);
        }
        else {
            ret = send(_sockfd, _buf.data(), _buf.size(), MSG_DONTWAIT);
            if (ret <= 0) {
                std::cerr << "Could not send packet to server" << std::endl;
                return false;
            }
            _buf.erase(_buf.begin(), _buf.begin() + ret);
        }
        if (_buf.empty()) {
             memset(&_event, 0, sizeof(_event));
            _event.data.fd = _sockfd;
            _event.events = EPOLLIN;
            if (epoll_ctl(_epollfd, EPOLL_CTL_MOD, _sockfd, &_event) == -1) {
                std::cerr << "Could not add socket to epoll" << std::endl;
                return false;
            }
        }
    }
    else if (_event.events & EPOLLIN) {
        this->_recv_len = recv(this->_sockfd, this->_recv_buffer, PIPE_BUF, MSG_DONTWAIT);
        if (_recv_len <= 0) {
            std::cerr << "Server disconnected" << std::endl;
            return false;
        }
        this->_buf.insert(this->_buf.end(), this->_recv_buffer, this->_recv_buffer + this->_recv_len);
        if (!this->_packetsize && !this->getPacketSize()) {
            std::cerr << "Invalid packet received from server" << std::endl;
            return false;
        }
        if (this->_packetsize) {
            if (this->_buf.size() >= this->_packetsize) {
                if (!this->decryptAESKey())
                    return false;
            this->_buf.clear();
            memset(&_event, 0, sizeof(_event));
            _event.data.fd = _sockfd;
            _event.events = EPOLLOUT | EPOLLIN;
            if (epoll_ctl(_epollfd, EPOLL_CTL_MOD, _sockfd, &_event) == -1) {
                std::cerr << "Could not modify socket event in epoll" << std::endl;
                return false;
            }
            this->_packetsize = 0;
            }
        }
    }
    else {
        std::cerr << "Server disconnected" << std::endl;
        return false;
    }
    return true;
}

bool    Client::secureReceive(bool& pending, bool& authenticated, bool& logs) {
    this->_recv_len = recv(this->_sockfd, this->_recv_buffer, PIPE_BUF, MSG_DONTWAIT);
    if (_recv_len <= 0) {
        std::cerr << "Server disconnected" << std::endl;
        return false;
    }
    this->_buf.insert(this->_buf.end(), this->_recv_buffer, this->_recv_buffer + this->_recv_len);
    if (!this->_packetsize && !this->getPacketSize()) {
        std::cerr << "Invalid packet received from server" << std::endl;
        return false;
    }
    if (this->_packetsize) {
        if (this->_buf.size() >= this->_packetsize) {
            if (!this->decrypt())
                return false;
            if (_secure && !authenticated) {
                if (this->_inbuf == "AUTH OK\n") {
                    authenticated = true;
                    std::cout << "Use command 'log?' to display the latest logs" << std::endl;
                }
                else if (this->_inbuf == "AUTH KO\n") {
                    std::cerr << "Authentication failed" << std::endl;
                }
                else {
                    std::cerr << "Authentication failed too many times, quitting Ben AFK" << std::endl;
                    return false;
                }
            }
            else {
                std::cout << this->_inbuf << std::endl;
                logs = false;
            }
            this->_buf.clear();
            this->_inbuf.clear();
            this->_packetsize = 0;
            pending = false;
            memset(&_event, 0, sizeof(_event));
            _event.data.fd = _sockfd;
            _event.events = EPOLLOUT | EPOLLIN;
            if (epoll_ctl(_epollfd, EPOLL_CTL_MOD, _sockfd, &_event) == -1) {
                std::cerr << "Could not modify socket event in epoll" << std::endl;
                return false;
            }
        }
    }
    return true;
}

bool    Client::secureSend(bool logs, bool quit, bool& pending, bool& authenticated) {
    if (!pending) {
        if (!this->encrypt())
            return false;
        std::string header = "Length ";
        header += std::to_string(this->_buf.size());
        header += "\n";
        this->_buf.insert(this->_buf.begin(), header.begin(), header.end());
        pending = true;
    }
    ssize_t ret = send(_sockfd, _buf.data(), _buf.size(), MSG_DONTWAIT);
    if (ret <= 0) {
        std::cerr << "Could not send packet to server, Matt_daemon is most probably disconnected" << std::endl;
        return false;
    }
    _buf.erase(_buf.begin(), _buf.begin() + ret);
    if (_buf.empty())
        pending = false;
    if (_buf.empty() && _secure && !authenticated) {
        memset(&_event, 0, sizeof(_event));
        _event.data.fd = _sockfd;
        _event.events = EPOLLIN;
        if (epoll_ctl(_epollfd, EPOLL_CTL_MOD, _sockfd, &_event) == -1) {
            std::cerr << "Could not modify socket event in epoll" << std::endl;
            return false;
        }
        pending = true;
    }
    else if (logs && _buf.empty() && _secure && authenticated) {
        memset(&_event, 0, sizeof(_event));
        _event.data.fd = _sockfd;
        _event.events = EPOLLIN;
        if (epoll_ctl(_epollfd, EPOLL_CTL_MOD, _sockfd, &_event) == -1) {
            std::cerr << "Could not modify socket event in epoll" << std::endl;
            return false;
        }
        pending = true;
    }
    else if (quit && _buf.empty()) {
        std::cout << "Goodbye" << std::endl;
        return false;
    }
    return true;
}

int Client::run(void) {
    bool        first = true;
    bool        logs = false;
    bool        quit = false;
    bool        authenticated = false;
    bool        pending = false;
    int         hasevent = 0;

    this->_buf.clear();
    this->_inbuf.clear();
    this->_packetsize = 0;
    if (connect(_sockfd, (struct sockaddr*)&_sockaddr, sizeof(_sockaddr)) == -1) {
        std::cerr << "Could not connect to server" << std::endl;
        return 1;
    }
    _epollfd = epoll_create(1);
    if (_epollfd <= 0) {
        std::cerr << "Could not create epoll fd" << std::endl;
        return 1;
    }
    memset(&_event, 0, sizeof(_event));
    _event.data.fd = _sockfd;
    _event.events = EPOLLIN;
    if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, _sockfd, &_event) == -1) {
        std::cerr << "Could not add socket to epoll" << std::endl;
        return 1;
    }
    while (true) {
        if (!(_secure && !_handshake) && _buf.empty() && !pending && !first) {
            if (_secure && !authenticated) {
                if (!this->getCredentials())
                    return 1;
            }
            else {
                if (!this->getUserInput(quit, logs))
                    return 0;
            }
        }
        hasevent = epoll_wait(_epollfd, &_event, 1, first ? 1000 : 1000*60*2);
        if (hasevent > 0) {
            if (_event.events & EPOLLHUP || _event.events & EPOLLERR) {
                std::cerr << "Server disconnected" << std::endl;
                return 1;
            }
            if (first) {
                std::cerr << "Server disconnected" << std::endl;
                return 1;
            }
            else if (_secure && !_handshake) {
                if (!this->handshake())
                    return 1;
            }
            else if (!_secure) {
                if (this->_buf.size() > PIPE_BUF) {
                    std::cerr << "You cannot send more than " << PIPE_BUF << " bytes to Matt_daemon" << std::endl;
                    return 1;
                }
                if (_event.events & EPOLLIN) {
                    std::cerr << "Server disconnected" << std::endl;
                    return 1;
                }
                ssize_t ret = send(_sockfd, _buf.data(), _buf.size(), MSG_DONTWAIT);
                if (ret <= 0) {
                    std::cerr << "Could not send packet to server" << std::endl;
                    return 1;
                }
                if (quit) {
                    std::cout << "Goodbye" << std::endl;
                    return 0;
                }
                _buf.erase(_buf.begin(), _buf.begin() + ret);
            }
            else if (_event.events & EPOLLOUT) {
                if (_event.events & EPOLLIN) {
                    std::cerr << "Server disconnected" << std::endl;
                    return 1;
                }
                if (!this->secureSend(logs, quit, pending, authenticated)) {
                    if (quit)
                        return 0;
                    return 1;
                }
            }
            else {
                if (!this->secureReceive(pending, authenticated, logs))
                    return 1;
            }
        }
        else if (first) {
            memset(&_event, 0, sizeof(_event));
            _event.data.fd = _sockfd;
            _event.events = EPOLLOUT | EPOLLIN;
            if (epoll_ctl(_epollfd, EPOLL_CTL_MOD, _sockfd, &_event) == -1) {
                std::cerr << "Could not add socket to epoll" << std::endl;
                return 1;
            }
            first = false;
        }
    }
    return 0;
}

bool    Client::decryptAESKey(void) {
        size_t    outlen;

        EVP_PKEY_CTX*     ctx = EVP_PKEY_CTX_new(this->_RSA_key.get(), nullptr);
        if (!ctx) {
            std::cerr << "Could not create context" << std::endl;
            return false;
        }
        if (EVP_PKEY_decrypt_init(ctx) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            std::cerr << "Could not init decryption" <<std::endl;
            return false;
        }
        if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            std::cerr << "Could not set padding" <<std::endl;
            return false;
        }
        if (EVP_PKEY_decrypt(ctx, nullptr, &outlen, (const unsigned char*) this->_buf.data(), this->_buf.size()) <= 0) {
            std::cerr << "Could not get length of decrypted result" << std::endl;
            EVP_PKEY_CTX_free(ctx);
            return false;
        }
        unsigned char* out_tmp_buf = nullptr;
        try {
            out_tmp_buf = new unsigned char[outlen];
        }
        catch (std::exception const & e) {
            std::cerr << "Allocation error" << std::endl;
            EVP_PKEY_CTX_free(ctx);
            return false;
        }
        if (EVP_PKEY_decrypt(ctx, out_tmp_buf, &outlen, (const unsigned char*) this->_buf.data(), this->_buf.size()) <= 0) {
            std::cerr << "Could not decrypt buffer" << std::endl;
            delete [] out_tmp_buf;
            EVP_PKEY_CTX_free(ctx);
            return false;
        }
    if (outlen != 48) {
        std::cerr << "Invalid key received, expected exactly 48 bytes, received " << outlen << std::endl; 
        return false;
    }
    memcpy(this->_key, out_tmp_buf, 32);
    memcpy(this->_iv, &out_tmp_buf[32], 16);
    delete [] out_tmp_buf;
    EVP_PKEY_CTX_free(ctx);
    this->_handshake = true;
    this->_buf.clear();
    return true;
}

bool    Client::encrypt(void) {
    int outlen, tmplen;
    unsigned char   nextiv[16];
    size_t gcm_ivlen = 16;
    unsigned char outbuf[PIPE_BUF + 128 + 16]; //maximum allowed size if PIPE_BUF, up to 128 bytes might be added during encryption (block size) and we need to add new iv
    unsigned char outtag[16];
    OSSL_PARAM params[2] = {OSSL_PARAM_END, OSSL_PARAM_END};

    if (this->_buf.size() > PIPE_BUF) {
        std::cerr << "You cannot send more than " << PIPE_BUF << " bytes to Matt_daemon" << std::endl;
        return false;
    }

    //Generate next iv
    if (!RAND_bytes(nextiv, 16)) {
        std::cerr << "Could not generate next iv" << std::endl;
        return false;
    }
    this->_buf.insert(this->_buf.end(), nextiv, nextiv + 16);
    params[0] = OSSL_PARAM_construct_size_t(OSSL_CIPHER_PARAM_AEAD_IVLEN, &gcm_ivlen);

    //Initialize context
    if (!EVP_EncryptInit_ex2(_ctx.get(), _cipher.get(), _key, _iv, params)) {
        std::cerr << "Could not initialize encryption" << std::endl;
        return false;
    }

    //Encrypt
    if (!EVP_EncryptUpdate(_ctx.get(), outbuf, &outlen, reinterpret_cast<unsigned char*>(_buf.data()), static_cast<int>(_buf.size()))) {
        std::cerr << "Could not encrypt buffer" << std::endl;
        return false;
    }

    //Finalize
    if (!EVP_EncryptFinal_ex(_ctx.get(), outbuf, &tmplen)) {
        std::cerr << "Could not finalize encryption" << std::endl;
        return false;
    }

    // Get tag
    params[0] = OSSL_PARAM_construct_octet_string(OSSL_CIPHER_PARAM_AEAD_TAG, outtag, 16);
    if (!EVP_CIPHER_CTX_get_params(_ctx.get(), params)) {
        std::cerr << "Could not get tag" << std::endl;
        return false;
    }

    //Add tag to the message
    _buf.assign(outbuf, outbuf + outlen);
    _buf.insert(_buf.end(), outtag, outtag + 16);

    //Save next iv
    memcpy(this->_iv, nextiv, 16);

    return true;
}

bool    Client::decrypt(void) {
    std::string tag;
    int outlen, rv;
    size_t gcm_ivlen = 16;
    unsigned char outbuf[PIPE_BUF + 128 + 16 + 1];
    OSSL_PARAM params[2] = {
        OSSL_PARAM_END, OSSL_PARAM_END
    };

    //Get tag
    if (this->_packetsize < 32) {
        std::cerr << "Packet size too small" << std::endl;
        return false;
    }
    tag.assign(this->_buf.begin() + this->_packetsize - 16, this->_buf.begin() + this->_packetsize);

    //Initialize context
    params[0] = OSSL_PARAM_construct_size_t(OSSL_CIPHER_PARAM_AEAD_IVLEN, &gcm_ivlen);
    if (!EVP_DecryptInit_ex2(_ctx.get(), _cipher.get(), _key, _iv, params)) {
        std::cerr << "Could not init decryption" << std::endl;
        return false;
    }

    //Decrypt plaintext
    if (!EVP_DecryptUpdate(_ctx.get(), outbuf, &outlen, reinterpret_cast<unsigned char*>(this->_buf.data()), static_cast<int>(this->_packetsize) - 16)) {
        std::cerr << "Could not decrypt package" << std::endl;
        return false;
    }

    //Set expected tag
    params[0] = OSSL_PARAM_construct_octet_string(OSSL_CIPHER_PARAM_AEAD_TAG, (void*)tag.data(), 16);
    if (!EVP_CIPHER_CTX_set_params(_ctx.get(), params)) {
        std::cerr << "Could not set expected tag" << std::endl;
        return false;
    }

    //Save next iv
    if (outlen < 16) {
        std::cerr << "Decrypted packet too small" << std::endl;
        return false;
    }
    memcpy(this->_iv, outbuf + outlen - 16, 16);

    //Add received message to buffer
    this->_inbuf.assign(outbuf, outbuf + outlen - 16);
    this->_buf.erase(this->_buf.begin(), this->_buf.begin() + this->_packetsize);

    //Verify tag
    rv = EVP_DecryptFinal_ex(_ctx.get(), outbuf, &outlen);
    if (rv <= 0) {
        std::cerr << "Tag authentication failed" << std::endl;
        return false;
    }

    return true;
}

void    Client::initRSA(void) {
    _RSA_key = std::shared_ptr<EVP_PKEY>(EVP_RSA_gen(4096), freeEVPPKey);
    if (!_RSA_key)
        throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not generate RSA key for handshake");
    _mem = std::shared_ptr<BIO>(BIO_new(BIO_s_mem()), freeBIO);
    if (!_mem)
        throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not initiate BIO");
    if (!PEM_write_bio_PUBKEY(_mem.get(), _RSA_key.get()))
        throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not put public key in BIO object");
    _pubkey_len = BIO_get_mem_data(_mem.get(), &_pubkey);
    if (_pubkey_len <= 0)
        throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not read public key from BIO");
    _ctx = std::shared_ptr<EVP_CIPHER_CTX>(EVP_CIPHER_CTX_new(), freeEVPCipherCtx);
    if (!_ctx)
        throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not create new context");
    _cipher = std::shared_ptr<EVP_CIPHER>(EVP_CIPHER_fetch(nullptr, "AES-256-GCM", nullptr), freeEVPCipher);
    if (!_cipher)
        throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not fetch cipher");
}
