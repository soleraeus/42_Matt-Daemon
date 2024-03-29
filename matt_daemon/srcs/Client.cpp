/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/28 20:21:03 by bdetune           #+#    #+#             */
/*   Updated: 2023/11/04 13:14:31 by bdetune          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

static const std::function<void(int*)>	closeOnDelete = [](int* ptr) -> void {
																				if (ptr != nullptr) {
																					if (*ptr > 0) {
																						close(*ptr);
                                                                                    }
																					delete ptr;
																				}
																			};

static const std::function<void(EVP_CIPHER_CTX*)> freeEvpCipherCtx = [](EVP_CIPHER_CTX* ptr) -> void { if (ptr != nullptr) {EVP_CIPHER_CTX_free(ptr);} };

static const std::function<void(EVP_CIPHER*)> freeEvpCipher = [](EVP_CIPHER* ptr) -> void { if (ptr != nullptr) {EVP_CIPHER_free(ptr);} };

Client::Client(void):
        _fd(nullptr, closeOnDelete),
        _packetsize(0),
        _secure(false),
        _handshake(false),
        _key(nullptr),
        _auth_tries(0),
        _authenticated(false),
        _ctx(nullptr, freeEvpCipherCtx),
        _cipher(nullptr, freeEvpCipher) {}

Client::Client(int fd):
        _fd(new int(fd), closeOnDelete),
        _packetsize(0),
        _secure(false),
        _handshake(false),
        _key(nullptr),
        _auth_tries(0),
        _authenticated(false),
        _ctx(nullptr, freeEvpCipherCtx),
        _cipher(nullptr, freeEvpCipher) {}

Client::Client(int fd, unsigned char* key, const std::string& username, const std::string& password):
        _fd(new int(fd), closeOnDelete),
        _packetsize(0),
        _secure(true),
        _handshake(false),
        _key(key),
        _username(username),
        _password(password),
        _auth_tries(0),
        _authenticated(false),
        _ctx(EVP_CIPHER_CTX_new(), freeEvpCipherCtx), 
        _cipher(EVP_CIPHER_fetch(nullptr, "AES-256-GCM", nullptr), freeEvpCipher) {
    if (!RAND_bytes(this->_iv, 16))
        throw std::system_error(std::error_code(), "Could not generate iv");
    if (!_ctx)
        throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not create new context");
    if (!_cipher)
        throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not fetch cipher");
}

Client::Client(const Client & src):
        _fd(src._fd),
        _packetsize(src._packetsize),
        _secure(src._secure),
        _handshake(src._handshake),
        _key(src._key),
        _username(src._username),
        _password(src._password),
        _auth_tries(src._auth_tries),
        _authenticated(src._authenticated),
        _buffer(src._buffer),
        _send_buffer(src._send_buffer),
        _encrypted_buffer(src._encrypted_buffer),
		_ctx(src._ctx),
        _cipher(src._cipher) {
    memcpy(this->_RSA_buf, src._RSA_buf, 1024);
    memcpy(this->_iv, src._iv, 16);
}
Client::Client(Client && src):
        _fd(std::move(src._fd)),
        _packetsize(src._packetsize),
        _secure(src._secure),
        _handshake(src._handshake),
        _key(src._key),
        _username(std::move(src._username)),
        _password(std::move(src._password)),
        _auth_tries(src._auth_tries),
        _authenticated(src._authenticated),
        _buffer(std::move(src._buffer)),
        _send_buffer(std::move(src._send_buffer)),
        _encrypted_buffer(std::move(src._encrypted_buffer)),
        _ctx(std::move(src._ctx)),
        _cipher(std::move(src._cipher)) {
    memcpy(this->_RSA_buf, src._RSA_buf, 1024);
    memcpy(this->_iv, src._iv, 16);
    src._key = nullptr;
    src._authenticated = false;
}

Client::~Client(void) {}

Client & Client::operator=(const Client & rhs)
{
	if (this == &rhs)
		return (*this);
	this->_fd = rhs._fd;
    this->_packetsize = rhs._packetsize;
    this->_secure = rhs._secure;
    this->_handshake = rhs._handshake;
    memcpy(this->_RSA_buf, rhs._RSA_buf, 1024);
    this->_key = rhs._key;
    memcpy(this->_iv, rhs._iv, 16);
    this->_username = rhs._username;
    this->_password = rhs._password;
    this->_auth_tries = rhs._auth_tries;
    this->_authenticated = rhs._authenticated;
    this->_buffer = rhs._buffer;
    this->_send_buffer = rhs._send_buffer;
    this->_encrypted_buffer = rhs._encrypted_buffer;
	this->_ctx = rhs._ctx;
    this->_cipher = rhs._cipher;
    return (*this);
}

Client & Client::operator=(Client && rhs)
{
	if (this == &rhs)
		return (*this);
	this->_fd = std::move(rhs._fd);
    this->_packetsize = rhs._packetsize;
    this->_secure = rhs._secure;
    this->_handshake = rhs._handshake;
    memcpy(this->_RSA_buf, rhs._RSA_buf, 1024);
    this->_key = rhs._key;
    memcpy(this->_iv, rhs._iv, 16);
    this->_username = rhs._username;
    this->_password = rhs._password;
    this->_auth_tries = rhs._auth_tries;
    this->_authenticated = rhs._authenticated;
    rhs._authenticated = false;
    this->_buffer = std::move(rhs._buffer);
    this->_send_buffer = std::move(rhs._send_buffer);
    this->_encrypted_buffer = std::move(rhs._encrypted_buffer);
    this->_ctx = std::move(rhs._ctx);
    this->_cipher = std::move(rhs._cipher);
    return (*this);
}

Client::Return  Client::getPacketSize(void)
{
    size_t  pos = 0;

    this->_packetsize = 0;
    for (size_t i = 0; i < this->_encrypted_buffer.size(); ++i)
    {
        if (this->_encrypted_buffer[i] == '\n')
        {
            if (i < 8 || i > 12
                || memcmp((void *)"Length ",(void *)&this->_encrypted_buffer[0], 7))
                return Client::Return::KICK;
            pos = i;
            if (this->_encrypted_buffer[i - 1] == '\r')
                pos = i - 1;
            if (pos == 7)
                return Client::Return::KICK;
            this->_encrypted_buffer[pos] = '\0';
            for (size_t j = 7; j < pos; ++j)
            {
                if (this->_encrypted_buffer[j] < '0' || this->_encrypted_buffer[j] > '9')
                    return Client::Return::KICK;
            }
            this->_packetsize = std::atoi(&this->_encrypted_buffer[7]);
            if (this->_packetsize > PIPE_BUF + 128 + 32)
                return Client::Return::KICK;
            this->_encrypted_buffer.erase(this->_encrypted_buffer.begin(), this->_encrypted_buffer.begin() + i + 1);
            return Client::Return::OK;
        }
        if ((this->_encrypted_buffer[i] < ' ' || this->_encrypted_buffer[i] > 'z')
            && this->_encrypted_buffer[i] != '\r')
            return Client::Return::KICK;
    }
    if (this->_encrypted_buffer.size() > 12)
        return Client::Return::KICK;
    return Client::Return::OK;
}

void    Client::trim(std::string& msg) {
    if (msg.size() == 0)
        return ;
    msg.erase(0, msg.find_first_not_of("\t\n\r\v\f "));
    if (msg.size() == 0)
        return ;
    std::string::size_type pos = msg.find_last_not_of("\t\n\r\v\f ");
    if (pos == msg.size() - 1)
        return ;
    msg.erase(pos + 1);
}

bool    Client::isValidUTF8(std::string& msg) {
    int             nb_char;
    unsigned char   current;
    unsigned int    val;

    for (std::string::size_type index = 0; index < msg.size(); ++index) {
        current = static_cast<unsigned char>(msg[index]);

        //Get number of bytes in current UTF8 character
        if ((current & 0x80) == 0) {
            nb_char = 1;
            val = current;
        }
        else if ((current & 0xE0) == 0xC0) {
            nb_char = 2;
            val = current & ~0xE0;
        }
        else if ((current & 0xF0) == 0xE0) {
            nb_char = 3;
            val = current & ~0xF0;
        }
        else if ((current & 0xF8) == 0xF0) {
            nb_char = 4;
            val = current & ~0xF8;
        }
        else
            return false ;

        if ((index + nb_char) > msg.size())
            return false ;

        //Retrieve full value of the code point
        for (int i = 1; i < nb_char; ++i) {
            ++index;
            current = static_cast<unsigned char>(msg[index]);
            if ((current & 0xC0) != 0x80)
                return false ;
            val = (val << 6) | (current & ~0xC0);
        }

        if (((val < ' ' && val != '\t') || val == 0x7F)                 //Disallow ascii non printable except tabs
            || val > 0x10FFFF                                           //Last valid UTF8 point
            || (val >= 0xD800 && val <= 0xDFFF)                         //Do not allow surrogates
            || (val <= 0x7F && nb_char != 1)                            //Next 4 lines are testing for overlong
            || (val >= 0x80 && val <= 0x7FF && nb_char != 2)
            || (val >= 0x800 && val <= 0xFFFF && nb_char != 3)
            || (val >= 0x10000 && val <= 0x10FFFF && nb_char != 4))
            return false ;
    }
    return true ;
}

Client::Return  Client::flush(std::shared_ptr<Tintin_reporter>& reporter)
{
    std::string             sub;
    std::string::size_type  pos;

    while ((pos = this->_buffer.find('\r')) != std::string::npos)
    {
        if (pos == this->_buffer.size() - 1)
            break;
        if (this->_buffer[pos + 1] != '\n')
            return Client::Return::KICK;
        this->_buffer.erase(pos, 1);
    }
    while ((pos = this->_buffer.find('\n')) != std::string::npos)
    {
        if (pos > PIPE_BUF)
            return Client::Return::KICK;
        sub = this->_buffer.substr(0, pos);
        if (sub == "quit")
            return Client::Return::QUIT;
        if (this->_authenticated && sub == "log?") {
            if (reporter->log("Client requested logs", Tintin_reporter::Loglevel::INFO) != Tintin_reporter::Return::OK
                || reporter->send_logs(this->_send_buffer) != Tintin_reporter::Return::OK)
                return Client::Return::QUIT;
            this->_buffer.clear();
            if (!this->encrypt(reporter))
                return Client::Return::KICK;
            this->addHeader();
            return Client::Return::SEND;
        }
        this->trim(sub);
        if (sub.size() == 0) {
            this->_buffer.erase(0, pos + 1);
            continue ;
        }
        if (!this->isValidUTF8(sub))
            return Client::Return::KICK;
        if (reporter->client_log(sub) != Tintin_reporter::Return::OK)
            return Client::Return::QUIT;
        this->_buffer.erase(0, pos + 1);
    }
    if (this->_buffer.size() > PIPE_BUF)
        return Client::Return::KICK;
    return Client::Return::OK;
}

Client::Return  Client::authenticate(std::shared_ptr<Tintin_reporter>& reporter) {
    this->_buffer.clear();
    if (!this->decrypt())
        return Client::Return::KICK;
    this->_auth_tries += 1;
    if (this->_buffer != (this->_username + "\n" + this->_password + "\n")) {
        if (this->_auth_tries >= 3)
            this->_send_buffer = "AUTH KO - TOO MANY TRIES\n";
        else
            this->_send_buffer = "AUTH KO\n";
        if (!this->encrypt(reporter))
            return Client::Return::KICK;
        this->addHeader();
        this->_buffer.clear();
        this->_packetsize = 0;
        return Client::Return::SEND;
    }
    this->_buffer.clear();
    this->_authenticated = true;
    this->_send_buffer = "AUTH OK\n";
    if (!this->encrypt(reporter))
        return Client::Return::KICK;
    this->addHeader();
    this->_packetsize = 0;
    return Client::Return::SEND;
}

Client::Return  Client::handshake(void) {
    if (this->_packetsize > 1023)
        return Client::Return::KICK;
    memcpy(this->_RSA_buf, this->_encrypted_buffer.data(), this->_packetsize);
    this->_RSA_buf[this->_packetsize] = '\0';
    this->_encrypted_buffer.erase(this->_encrypted_buffer.begin(), this->_encrypted_buffer.begin() + this->_packetsize);
    return this->sendKey();
}

Client::Return  Client::receive(std::shared_ptr<Tintin_reporter>& reporter)
{
    ssize_t                 len = 0;
    Client::Return          ret;
    
    len = recv(*(this->_fd), (void *)this->_recv_buffer, PIPE_BUF, MSG_DONTWAIT);
    if (len <= 0) {
        return Client::Return::KICK;
    }
    if (this->_secure)
    {
        this->_encrypted_buffer.insert(this->_encrypted_buffer.end(), this->_recv_buffer, this->_recv_buffer + len);
        if (!this->_packetsize && (ret = this->getPacketSize()) != Client::Return::OK)
            return ret;
        while (this->_packetsize > 0 && this->_encrypted_buffer.size() >= static_cast<size_t>(this->_packetsize))
        {
            if (!this->_handshake)
                return this->handshake();
            if (!this->_authenticated)
                return this->authenticate(reporter);
            if (!this->decrypt())
                return Client::Return::KICK;
            if ((ret = this->getPacketSize()) != Client::Return::OK)
                return ret;
        }
    }
    else
        this->_buffer.insert(this->_buffer.end(), this->_recv_buffer, &this->_recv_buffer[len]);
    return this->flush(reporter);
}

Client::Return  Client::sendKey(void) {
    size_t  outlen;

    //Putting public key received in BIO
    std::unique_ptr<BIO, std::function<void(BIO*)>>    pubKey(BIO_new_mem_buf((void*) this->_RSA_buf, this->_packetsize),
                                                               [](BIO* ptr) -> void {
                                                                    if (ptr != nullptr)
                                                                        BIO_free(ptr);
                                                                });
    if (!pubKey)
        return Client::Return::KICK;
    this->_packetsize = 0;

    //Using the BIO to create an EVP_KEY
    EVP_PKEY* RSA_key = nullptr;
    RSA_key = PEM_read_bio_PUBKEY(pubKey.get(), &RSA_key, nullptr, nullptr);
    if (!RSA_key) {
        return Client::Return::KICK;
    }
    std::unique_ptr<EVP_PKEY, std::function<void(EVP_PKEY*)>>    RSAKeyPtr(RSA_key,
                                                                            [](EVP_PKEY* ptr) -> void {
                                                                                if (ptr != nullptr)
                                                                                    EVP_PKEY_free(ptr);
                                                                            });
    std::unique_ptr<EVP_PKEY_CTX, std::function<void(EVP_PKEY_CTX*)>>   ctx(EVP_PKEY_CTX_new(RSAKeyPtr.get(), nullptr),
                                                                            [](EVP_PKEY_CTX* ptr) -> void {
                                                                                if (ptr != nullptr)
                                                                                    EVP_PKEY_CTX_free(ptr);
                                                                            });
    if (!ctx)
      return Client::Return::KICK;
    if (EVP_PKEY_encrypt_init(ctx.get()) <= 0) {
      return Client::Return::KICK;
    }
    if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_OAEP_PADDING) <= 0) {
      return Client::Return::KICK;
    }
    std::string out(this->_key, this->_key + 32);
    out.insert(out.end(), this->_iv, this->_iv + 16);
    if (EVP_PKEY_encrypt(ctx.get(), nullptr, &outlen, (const unsigned char*) out.data(), out.size()) <= 0) {
      return Client::Return::KICK;
    }
    unsigned char* out_tmp_buf = nullptr; 
    try {
      out_tmp_buf = new unsigned char[outlen];
    }
    catch (std::exception const & e) {
      return Client::Return::KICK;
    }
    if (EVP_PKEY_encrypt(ctx.get(), out_tmp_buf, &outlen, (const unsigned char*) out.data(), out.size()) <= 0) {
      delete [] out_tmp_buf;
      return Client::Return::KICK;
    }
    this->_send_buffer.insert(this->_send_buffer.end(), out_tmp_buf, out_tmp_buf + outlen);
    this->addHeader();
    delete [] out_tmp_buf;
    this->_handshake = true;
    return Client::Return::SEND;
}

Client::Return    Client::send(void) {
    size_t ret = ::send(*(this->_fd), (void*) this->_send_buffer.data(), this->_send_buffer.size(), MSG_DONTWAIT);
   if (ret <= 0) {
       return Client::Return::KICK;
   }
   if (ret == this->_send_buffer.size()) {
       this->_send_buffer.clear();
       if (this->_auth_tries >= 3 && !this->_authenticated)
           return Client::Return::KICK;
       return Client::Return::OK;
   }
   this->_send_buffer.erase(0, ret);
   return Client::Return::SEND;
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
    if (this->_packetsize < 16) {
        return false;
    }
    tag.assign(this->_encrypted_buffer.begin() + this->_packetsize - 16, this->_encrypted_buffer.begin() + this->_packetsize);

    //Initialize decryption
    params[0] = OSSL_PARAM_construct_size_t(OSSL_CIPHER_PARAM_AEAD_IVLEN, &gcm_ivlen);
    if (!EVP_DecryptInit_ex2(_ctx.get(), _cipher.get(), _key, _iv, params)) {
        return false;
    }

    // Decrypt
    if (!EVP_DecryptUpdate(_ctx.get(), outbuf, &outlen, reinterpret_cast<unsigned char*>(this->_encrypted_buffer.data()), this->_packetsize - 16)) {
        return false;
    }

    //Set expected tag
    params[0] = OSSL_PARAM_construct_octet_string(OSSL_CIPHER_PARAM_AEAD_TAG, (void*)tag.data(), 16);
    if (!EVP_CIPHER_CTX_set_params(_ctx.get(), params)) {
        return false;
    }

    //Save new IV and add decrypted packet to buffer for logging
    if (outlen < 16) {
        return false;
    }
    memcpy(this->_iv, outbuf + outlen - 16, 16);
    this->_buffer.insert(this->_buffer.end(), outbuf, outbuf + outlen - 16);
    this->_encrypted_buffer.erase(this->_encrypted_buffer.begin(), this->_encrypted_buffer.begin() + this->_packetsize);

    //Verify tag
    rv = EVP_DecryptFinal_ex(_ctx.get(), outbuf, &outlen);
    if (rv <= 0) {
        return false;
    }
    return true;
}

bool    Client::encrypt(std::shared_ptr<Tintin_reporter>& reporter) {
    int outlen, tmplen;
    unsigned char   nextiv[16];
    size_t gcm_ivlen = 16;
    unsigned char outbuf[PIPE_BUF + 128 + 16]; //maximum allowed size if PIPE_BUF, up to 128 bytes might be added during encryption (block size) and we need to add new iv
    unsigned char outtag[16];
    OSSL_PARAM params[2] = {OSSL_PARAM_END, OSSL_PARAM_END};

    //Handle next iv
    if (!RAND_bytes(nextiv, 16)) {
        if (reporter->log("Could not generate next iv for client secure connection", Tintin_reporter::Loglevel::ERROR) != Tintin_reporter::Return::OK) {}
        return false;
    }
    this->_send_buffer.insert(this->_send_buffer.end(), nextiv, nextiv + 16);
    params[0] = OSSL_PARAM_construct_size_t(OSSL_CIPHER_PARAM_AEAD_IVLEN, &gcm_ivlen);

    //Initialize encryption
    if (!EVP_EncryptInit_ex2(_ctx.get(), _cipher.get(), _key, _iv, params)) {
        if (reporter->log("Could not initialize encryption for client secure connection", Tintin_reporter::Loglevel::ERROR) != Tintin_reporter::Return::OK) {}
        return false;
    }

    //Encrypt
    if (!EVP_EncryptUpdate(_ctx.get(), outbuf, &outlen, reinterpret_cast<unsigned char*>(_send_buffer.data()), static_cast<int>(_send_buffer.size()))) {
        if (reporter->log("Could not encrypt buffer for client secure connection", Tintin_reporter::Loglevel::ERROR) != Tintin_reporter::Return::OK) {}
        return false;
    }
    if (!EVP_EncryptFinal_ex(_ctx.get(), outbuf, &tmplen)) {
        if (reporter->log("Could not finalize encryption for client secure connection", Tintin_reporter::Loglevel::ERROR) != Tintin_reporter::Return::OK) {}
        return false;
    }

    //Get tag
    params[0] = OSSL_PARAM_construct_octet_string(OSSL_CIPHER_PARAM_AEAD_TAG, outtag, 16);
    if (!EVP_CIPHER_CTX_get_params(_ctx.get(), params)) {
        if (reporter->log("Could not get tag", Tintin_reporter::Loglevel::ERROR) != Tintin_reporter::Return::OK) {}
        return false;
    }

    //Put encrypted string and tag in send buffer
    this->_send_buffer.assign(outbuf, outbuf + outlen);
    this->_send_buffer.insert(this->_send_buffer.end(), outtag, outtag + 16);

    //Save new iv
    memcpy(this->_iv, nextiv, 16);

    return true;
}

inline void    Client::addHeader(void) {
    std::string header = "Length " + std::to_string(this->_send_buffer.size()) + "\n";
    this->_send_buffer.insert(this->_send_buffer.begin(), header.begin(), header.end());
}
