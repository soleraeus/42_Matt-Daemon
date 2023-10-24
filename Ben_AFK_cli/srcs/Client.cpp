/* ************************************************************************** */
/*                                                                                                                                                        */
/*                                                                                                                :::            ::::::::     */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                                                                        +:+ +:+                 +:+         */
/*     By: bdetune <marvin@42.fr>                                         +#+    +:+             +#+                */
/*                                                                                                +#+#+#+#+#+     +#+                     */
/*     Created: 2023/10/19 20:55:26 by bdetune                     #+#        #+#                         */
/*   Updated: 2023/10/24 21:34:22 by bdetune          ###   ########.fr       */
/*                                                                                                                                                        */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(const char* ip, bool secure): _secure(secure), _handshake(false), _sockfd(0), _RSA_key(nullptr), _mem(nullptr), _pubkey(nullptr), _pubkey_len(0), _epollfd(0), _ctx(nullptr), _cipher(nullptr) {
    if (secure) {
        _RSA_key = EVP_RSA_gen(4096);
        if (!_RSA_key) {
            throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not generate RSA key for handshake");
        }
        _mem = BIO_new(BIO_s_mem());
        if (!_mem) {
            EVP_PKEY_free(_RSA_key);
            throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not initiate BIO");
        }
        if (!PEM_write_bio_PUBKEY(_mem, _RSA_key)) {
            EVP_PKEY_free(_RSA_key);
            BIO_free(_mem);
            throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not put public key in BIO object");
        }
        _pubkey_len = BIO_get_mem_data(_mem, &_pubkey);
        if (_pubkey_len <= 0) {
            EVP_PKEY_free(_RSA_key);
            BIO_free(_mem);
            throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not read public key from BIO");
        }
        if ((_ctx = EVP_CIPHER_CTX_new()) == nullptr) {
            EVP_PKEY_free(_RSA_key);
            BIO_free(_mem);
            throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not create new context");
        }
        if ((_cipher = EVP_CIPHER_fetch(nullptr, "AES-256-GCM", nullptr)) == nullptr) {
            EVP_PKEY_free(_RSA_key);
            BIO_free(_mem);
            EVP_CIPHER_CTX_free(_ctx);
            throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not fetch cipher");
        }
    }
    bzero(&_sockaddr, sizeof(_sockaddr));
    if ((_sockfd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0)) <= 0) {
        if (_RSA_key) {
            EVP_CIPHER_CTX_free(_ctx);
            EVP_CIPHER_free(_cipher);
            BIO_free(_mem);
            EVP_PKEY_free(_RSA_key);
        }
        throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not create socket");
    }
    _sockaddr.sin_family = AF_INET;
        _sockaddr.sin_port = secure ? htons(4343) : htons(4242);
    if (inet_pton(AF_INET, ip ? ip : "127.0.0.1", &_sockaddr.sin_addr) <= 0) {
        if (_RSA_key) {
            EVP_CIPHER_CTX_free(_ctx);
            EVP_CIPHER_free(_cipher);
            BIO_free(_mem);
            EVP_PKEY_free(_RSA_key);
        }
        close(_sockfd);
        throw std::system_error(std::make_error_code(std::errc::bad_address), "Bad ip address provided");
    }
}

Client::Client(const Client& src): _secure(src._secure), _handshake(src._handshake), _sockfd(0), _RSA_key(nullptr), _mem(nullptr), _pubkey(nullptr), _pubkey_len(0), _epollfd(0), _ctx(nullptr), _cipher(nullptr) {
    if (src._RSA_key) {
        _RSA_key = EVP_PKEY_dup(src._RSA_key);
        if (!_RSA_key) {
            throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not duplicate key");
        }
        _mem = BIO_new(BIO_s_mem());
        if (!_mem) {
            EVP_PKEY_free(_RSA_key);
            throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not initiate BIO");
        }
        if (!PEM_write_bio_PUBKEY(_mem, _RSA_key)) {
            EVP_PKEY_free(_RSA_key);
            BIO_free(_mem);
            throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not put public key in BIO object");
        }
        _pubkey_len = BIO_get_mem_data(_mem, &_pubkey);
        if (_pubkey_len <= 0) {
            EVP_PKEY_free(_RSA_key);
            BIO_free(_mem);
            throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not read public key from BIO");
        }
        if ((_ctx = EVP_CIPHER_CTX_new()) == nullptr) {
            EVP_PKEY_free(_RSA_key);
            BIO_free(_mem);
            throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not create new context");
        }
        if ((_cipher = EVP_CIPHER_fetch(nullptr, "AES-256-GCM", nullptr)) == nullptr) {
            EVP_PKEY_free(_RSA_key);
            BIO_free(_mem);
            EVP_CIPHER_CTX_free(_ctx);
            throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not fetch cipher");
        }
    }
    memcpy((void *)&_sockaddr, (void *)&src._sockaddr, sizeof(_sockaddr));
    if (src._sockfd > 0) {
        _sockfd = dup(src._sockfd);
        if (_sockfd <= 0) {
            if (_RSA_key) {
                EVP_CIPHER_CTX_free(_ctx);
                EVP_CIPHER_free(_cipher);
                BIO_free(_mem);
                EVP_PKEY_free(_RSA_key);
             }
             throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not duplicate socket");
        }
    }
    if (src._epollfd > 0) {
        _epollfd = dup(src._epollfd);
        if (_epollfd <= 0) {
            if (_RSA_key) {
                EVP_CIPHER_CTX_free(_ctx);
                EVP_CIPHER_free(_cipher);
                BIO_free(_mem);
                EVP_PKEY_free(_RSA_key);
            }
            if (_sockfd > 0)
                close(_sockfd);
            throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not duplicate epoll fd");
        }
    }
}

Client::Client(Client&& src): _secure(src._secure), _handshake(src._handshake), _sockfd(src._sockfd), _RSA_key(src._RSA_key), _mem(src._mem), _pubkey(src._pubkey), _pubkey_len(src._pubkey_len), _epollfd(src._epollfd), _ctx(src._ctx), _cipher(src._cipher) {
    memcpy((void *)&_sockaddr, (void *)&src._sockaddr, sizeof(_sockaddr));
    src._sockfd = 0;
    src._RSA_key = nullptr;
    src._mem = nullptr;
    src._pubkey = nullptr;
    src._epollfd = 0;
    src._ctx = nullptr;
    src._cipher = nullptr;
}

Client::~Client(void) {
    if (_sockfd > 0)
        close(_sockfd);
    if (_RSA_key)
        EVP_PKEY_free(_RSA_key);
    if (_mem)
        BIO_free(_mem);
    if (_epollfd > 0)
        close(_epollfd);
    if (_ctx)
        EVP_CIPHER_CTX_free(_ctx);
    if (_cipher)
        EVP_CIPHER_free(_cipher);
}

Client& Client::operator=(const Client& rhs) {
    if (this == &rhs)
        return (*this);
    _secure = rhs._secure;
    _handshake = rhs._handshake;
    if (_sockfd > 0) {
        close(_sockfd);
        _sockfd = 0;
    }
    if (_RSA_key) {
        EVP_PKEY_free(_RSA_key);
        _RSA_key = nullptr;
    }
    if (_mem) {
        BIO_free(_mem);
        _mem = nullptr;
    }
    if (_ctx) {
        EVP_CIPHER_CTX_free(_ctx);
        _ctx = nullptr;
    }
    if (_cipher) {
        EVP_CIPHER_free(_cipher);
        _cipher = nullptr;
    }
    if (rhs._RSA_key) {
        _RSA_key = EVP_PKEY_dup(rhs._RSA_key);
        if (!_RSA_key) {
            throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not duplicate key");
        }
        _mem = BIO_new(BIO_s_mem());
        if (!_mem) {
            EVP_PKEY_free(_RSA_key);
            throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not initiate BIO");
        }
        if (!PEM_write_bio_PUBKEY(_mem, _RSA_key)) {
            EVP_PKEY_free(_RSA_key);
            BIO_free(_mem);
            throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not put public key in BIO object");
        }
        _pubkey_len = BIO_get_mem_data(_mem, &_pubkey);
        if (_pubkey_len <= 0) {
            EVP_PKEY_free(_RSA_key);
            BIO_free(_mem);
            throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not read public key from BIO");
        }
        if ((_ctx = EVP_CIPHER_CTX_new()) == nullptr) {
            EVP_PKEY_free(_RSA_key);
            BIO_free(_mem);
            throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not create new context");
        }
        if ((_cipher = EVP_CIPHER_fetch(nullptr, "AES-256-GCM", nullptr)) == nullptr) {
            EVP_PKEY_free(_RSA_key);
            BIO_free(_mem);
            EVP_CIPHER_CTX_free(_ctx);
            throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not fetch cipher");
        }
    }
    memcpy((void *)&_sockaddr, (void *)&rhs._sockaddr, sizeof(_sockaddr));
    if (rhs._sockfd >= 0) {
        _sockfd = dup(rhs._sockfd);
        if (_sockfd <= 0) {
            if (_RSA_key) {
                EVP_CIPHER_CTX_free(_ctx);
                EVP_CIPHER_free(_cipher);
                BIO_free(_mem);
                EVP_PKEY_free(_RSA_key);
            }
            throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not duplicate socket");
        }
    }
    if (rhs._epollfd > 0) {
        _epollfd = dup(rhs._epollfd);
        if (_epollfd <= 0) {
            if (_RSA_key) {
                EVP_CIPHER_CTX_free(_ctx);
                EVP_CIPHER_free(_cipher);
                BIO_free(_mem);
                EVP_PKEY_free(_RSA_key);
            }
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
    if (_RSA_key) {
        EVP_PKEY_free(_RSA_key);
        _RSA_key = nullptr;
    }
    if (_mem) {
        BIO_free(_mem);
        _mem = nullptr;
    }
    if (_ctx) {
        EVP_CIPHER_CTX_free(_ctx);
        _ctx = nullptr;
    }
    if (_cipher) {
        EVP_CIPHER_free(_cipher);
        _cipher = nullptr;
    }
    _secure = rhs._secure;
    _handshake = rhs._handshake;
    _sockfd = rhs._sockfd;
    rhs._sockfd = 0;
    _RSA_key = rhs._RSA_key;
    rhs._RSA_key = nullptr;
    _mem = rhs._mem;
    rhs._mem = nullptr;
    _pubkey = rhs._pubkey;
    rhs._pubkey = nullptr;
    _pubkey_len = rhs._pubkey_len;
    memcpy((void *)&_sockaddr, (void *)&rhs._sockaddr, sizeof(_sockaddr));
    _epollfd = rhs._epollfd;
    rhs._epollfd = 0;
    _ctx = rhs._ctx;
    rhs._ctx = nullptr;
    _cipher = rhs._cipher;
    rhs._cipher = nullptr;
    return (*this);
}

void    Client::printPubkey(void)
{
    if (!_RSA_key || !_pubkey) {
        std::cerr << "No key generated" << std::endl;
    }
    else {
        for (long i = 0; i < _pubkey_len; ++i) {
            std::cout << _pubkey[i];
        }
    }
}

bool    Client::getPacketSize(void)
{
    size_t    pos = 0;

    this->_packetsize = 0;
    for (size_t i = 0; i < this->_buf.size(); ++i)
    {
        if (this->_buf[i] == '\n')
        {
            if (i < 8 || i > 12)
                return false;
            if (memcmp((void *)"Length ",(void *)&this->_buf[0], 7))
                return false;
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
            if (this->_packetsize > PIPE_BUF)
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


int Client::run(void) {
    int         hasevent = 0;
    ssize_t     ret = 0;

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
    _event.events = EPOLLOUT;
    if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, _sockfd, &_event) == -1) {
        std::cerr << "Could not add socket to epoll" << std::endl;
        return 1;
    }
    while (true) {
        if (!(_secure && !_handshake) && _buf.empty()) {
            std::getline(std::cin, _buf);
            if (std::cin.eof()) {
                std::cout << "Goodbye" << std::endl;
                epoll_ctl(_epollfd, EPOLL_CTL_DEL, _sockfd, &_event);
                return 0;
            }
            std::cerr << "Written: " << _buf << std::endl;
        }
        hasevent = epoll_wait(_epollfd, &_event, 1, 1000*60*2);
        if (hasevent > 0) {
            if (_event.events & EPOLLHUP) {
                std::cerr << "Server disconnected" << std::endl;
                return 1;
            }
            if (_secure && !_handshake) {
                if (_event.events & EPOLLOUT) {
                    if (_buf.empty()) {
                        _buf = "Length " + std::to_string(_pubkey_len);
                        _buf += "\n";
                        _buf.insert(_buf.end(), _pubkey, _pubkey + _pubkey_len);
                        ret = send(_sockfd, _buf.data(), _buf.size(), MSG_DONTWAIT);
                        if (ret <= 0) {
                            std::cerr << "Could not send packet to server" << std::endl;
                            return 1;
                        }
                        _buf.erase(_buf.begin(), _buf.begin() + ret);
                    }
                    else {
                        ret = send(_sockfd, _buf.data(), _buf.size(), MSG_DONTWAIT);
                        if (ret <= 0) {
                            std::cerr << "Could not send packet to server" << std::endl;
                            return 1;
                        }
                        _buf.erase(_buf.begin(), _buf.begin() + ret);
                    }
                    if (_buf.empty()) {
                        memset(&_event, 0, sizeof(_event));
                        _event.data.fd = _sockfd;
                        _event.events = EPOLLIN;
                        std::cerr << "Waiting to receive symmetric key" << std::endl;
                        if (epoll_ctl(_epollfd, EPOLL_CTL_MOD, _sockfd, &_event) == -1) {
                            std::cerr << "Could not add socket to epoll" << std::endl;
                            return 1;
                        }
                    }
                }
                else if (_event.events & EPOLLIN) {
                    this->_recv_len = recv(this->_sockfd, this->_recv_buffer, PIPE_BUF, MSG_DONTWAIT);
                    if (_recv_len <= 0) {
                        std::cerr << "Server disconnected" << std::endl;
                        return 1;
                    }
                    this->_buf.insert(this->_buf.end(), this->_recv_buffer, this->_recv_buffer + this->_recv_len);
                    if (!this->_packetsize && !this->getPacketSize()) {
                        std::cerr << "Invalid packet received from server" << std::endl;
                        return 1;
                    }
                    if (this->_packetsize) {
                        if (this->_buf.size() >= this->_packetsize) {
                            std::cerr << "Received key from Server" << std::endl;
                            for (size_t i = 0; i < this->_buf.size(); ++i) {
                                std::cerr << std::setw(2) << std::setfill('0') << std::hex << (int)this->_buf[i];
                            }
                            std::cerr << std::endl;
                            if (!this->decryptAESKey())
                                return 1;
                            memset(&_event, 0, sizeof(_event));
                            _event.data.fd = _sockfd;
                            _event.events = EPOLLOUT;
                            if (epoll_ctl(_epollfd, EPOLL_CTL_MOD, _sockfd, &_event) == -1) {
                                std::cerr << "Could not modify socket event in epoll" << std::endl;
                                return 1;
                            }
                        }
                    }
                }
                else {
                    std::cerr << "Server disconnected" << std::endl;
                    return 1;
                }
            }
            else if (!_secure) {
                _buf += "\n";
                ret = send(_sockfd, _buf.data(), _buf.size(), MSG_DONTWAIT);
                if (ret <= 0) {
                    std::cerr << "Could not send packet to server" << std::endl;
                    return 1;
                }
                _buf.erase(_buf.begin(), _buf.begin() + ret);
            }
            else {
                if (!this->encrypt())
                    return 1;
                std::string header = "Length ";
                header += std::to_string(this->_buf.size());
                header += "\n";
                this->_buf.insert(this->_buf.begin(), header.begin(), header.end());
                ret = send(_sockfd, _buf.data(), _buf.size(), MSG_DONTWAIT);
                if (ret <= 0) {
                    std::cerr << "Could not send packet to server, Matt_daemon is most probably disconnected" << std::endl;
                    return 1;
                }
                _buf.erase(_buf.begin(), _buf.begin() + ret);
            }
        }
    }
    return 0;
}

bool    Client::decryptAESKey(void) {
        size_t    outlen;

        EVP_PKEY_CTX*     ctx = EVP_PKEY_CTX_new(this->_RSA_key, nullptr);
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
        std::cerr << "Payload is " << outlen << " bytes" << std::endl;
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
        std::cerr << "Received key" << std::endl;
        for (int i = 0; i < 32; ++i) {
            std::cerr << std::setw(2) << std::setfill('0') << std::hex << (int)this->_key[i];
        }
        std::cerr << std::endl << "Received iv" << std::endl;
        for (int i = 0; i < 16; ++i) {
            std::cerr << std::setw(2) << std::setfill('0') << std::hex << (int)this->_iv[i];
        }
    std::cerr << std::dec << std::endl;
        delete [] out_tmp_buf;
        EVP_PKEY_CTX_free(ctx);
    std::cerr << "Succesfully received AES key" << std::endl;
    this->_handshake = true;
    this->_buf.clear();
    return true;
}

bool    Client::encrypt(void) {
    int outlen, tmplen;
    size_t gcm_ivlen = 16;
    unsigned char outbuf[PIPE_BUF + 128 + 1]; //maximum allowed size if PIPE_BUF and up to 128 bytes might be added during encryption (block size)
    unsigned char outtag[16];
    OSSL_PARAM params[2] = {OSSL_PARAM_END, OSSL_PARAM_END};

    this->_buf += '\n';
    if (this->_buf.size() > PIPE_BUF) {
        std::cerr << "You cannot send more than " << PIPE_BUF << " bytes to Matt_daemon" << std::endl;
        return false;
    }
    std::cerr << "AES GCM Encrypt:\nPlaintext:\n" << this->_buf << std::endl;
    params[0] = OSSL_PARAM_construct_size_t(OSSL_CIPHER_PARAM_AEAD_IVLEN, &gcm_ivlen);

    /*
     * Initialise an encrypt operation with the cipher/mode, key, IV and
     * IV length parameter.
     */
    if (!EVP_EncryptInit_ex2(_ctx, _cipher, _key, _iv, params)) {
        std::cerr << "Could not initialize encryption" << std::endl;
        return false;
    }

    /* Encrypt plaintext */
    if (!EVP_EncryptUpdate(_ctx, outbuf, &outlen, reinterpret_cast<unsigned char*>(_buf.data()), static_cast<int>(_buf.size()))) {
        std::cerr << "Could not encrypt buffer" << std::endl;
        return false;
    }
    /* Output encrypted block */
    printf("Ciphertext:\n");
    BIO_dump_fp(stderr, outbuf, outlen);

    std::cerr << "Size after first encryption: " << outlen << std::endl;
    /* Finalise: note get no output for GCM */
    if (!EVP_EncryptFinal_ex(_ctx, outbuf, &tmplen)) {
        std::cerr << "Could not finalize encryption" << std::endl;
        return false;
    }
    std::cerr << "Size after final encryption: " << tmplen << std::endl;

    /* Get tag */
    params[0] = OSSL_PARAM_construct_octet_string(OSSL_CIPHER_PARAM_AEAD_TAG, outtag, 16);
    if (!EVP_CIPHER_CTX_get_params(_ctx, params)) {
        std::cerr << "Could not get tag" << std::endl;
        return false;
    }

    /* Output tag */
    std::cerr << "Tag:" << std::endl;;
    BIO_dump_fp(stderr, outtag, 16);
    _buf.assign(outbuf, outbuf + outlen);
    std::cerr << "Encrypted:" << std::endl;;
    BIO_dump_fp(stderr, _buf.data(), static_cast<int>(_buf.size()));
    std::cerr << std::endl;
    _buf.insert(_buf.end(), outtag, outtag + 16);
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

    printf("AES GCM Decrypt:\n");
    printf("Ciphertext:\n");
    BIO_dump_fp(stderr, _inbuf.data(), static_cast<int>(_inbuf.size()));

    if (_inbuf.size() < 16) {
        std::cerr << "Tag not provided correctly" << std::endl;
        return false;
    }
    tag.assign(_inbuf.end() - 16, _inbuf.end());
    std::cerr << "tag:" << std::endl;
    BIO_dump_fp(stderr, tag.data(), static_cast<int>(tag.size()));
    _inbuf.erase(_inbuf.end() - 16, _inbuf.end());
    std::cerr << "New buf:" << std::endl;
    BIO_dump_fp(stderr, _inbuf.data(), static_cast<int>(_inbuf.size()));

    params[0] = OSSL_PARAM_construct_size_t(OSSL_CIPHER_PARAM_AEAD_IVLEN, &gcm_ivlen);

    /*
     * Initialise an encrypt operation with the cipher/mode, key, IV and
     * IV length parameter.
     */
    if (!EVP_DecryptInit_ex2(_ctx, _cipher, _key, _iv, params)) {
        std::cerr << "Could not init decryption" << std::endl;
        return false;
    }

    /* Decrypt plaintext */
    if (!EVP_DecryptUpdate(_ctx, outbuf, &outlen, reinterpret_cast<unsigned char*>(_inbuf.data()), static_cast<int>(_inbuf.size()))) {
        std::cerr << "Could not decrypt string" << std::endl;
        return false;
    }

    /* Output decrypted block */
    printf("Plaintext:\n");
    BIO_dump_fp(stdout, outbuf, outlen);

    /* Set expected tag value. */
    params[0] = OSSL_PARAM_construct_octet_string(OSSL_CIPHER_PARAM_AEAD_TAG, (void*)tag.data(), 16);

    if (!EVP_CIPHER_CTX_set_params(_ctx, params)) {
        std::cerr << "Could not get params" << std::endl;
        return false;
    }

    std::cerr << "Current length" << outlen << std::endl;

    /* Finalise: note get no output for GCM */
    rv = EVP_DecryptFinal_ex(_ctx, outbuf, &outlen);

    std::cerr << "Length after final encrypt" << outlen << std::endl;

    /*
     * Print out return value. If this is not successful authentication
     * failed and plaintext is not trustworthy.
     */
    if (rv <= 0) {
        std::cerr << "Incorrect tag provided, data might have been corrupted" << std::endl;
        return false;
    }
    std::cerr << "Tag verified" << std::endl;
    return true;
}
