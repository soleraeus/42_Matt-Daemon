/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/19 20:55:26 by bdetune           #+#    #+#             */
/*   Updated: 2023/10/23 22:32:23 by bdetune          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(const char* ip, bool secure): _secure(secure), _handshake(false), _sockfd(0), _RSA_key(NULL), _mem(NULL), _pubkey(NULL), _pubkey_len(0), _epollfd(0) {
	if (secure)	{
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
	}
	bzero(&_sockaddr, sizeof(_sockaddr));
	if ((_sockfd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0)) <= 0) {
		EVP_PKEY_free(_RSA_key);
		BIO_free(_mem);
		throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not create socket");
	}
	_sockaddr.sin_family = AF_INET;
    _sockaddr.sin_port = secure ? htons(4343) : htons(4242);
	if (inet_pton(AF_INET, ip ? ip : "127.0.0.1", &_sockaddr.sin_addr) <= 0) {
		if (_RSA_key)
			EVP_PKEY_free(_RSA_key);
		if (_mem)
			BIO_free(_mem);
		close(_sockfd);
		throw std::system_error(std::make_error_code(std::errc::bad_address), "Bad ip address provided");
	}
}

Client::Client(const Client& src): _secure(src._secure), _handshake(src._handshake), _sockfd(0), _RSA_key(NULL), _mem(NULL), _pubkey(NULL), _pubkey_len(0), _epollfd(0) {
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
	}
	memcpy((void *)&_sockaddr, (void *)&src._sockaddr, sizeof(_sockaddr));
	if (src._sockfd > 0) {
		_sockfd = dup(src._sockfd);
		if (_sockfd <= 0) {
			if (_RSA_key)
				EVP_PKEY_free(_RSA_key);
			if (_mem)
				BIO_free(_mem);
			throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not duplicate socket");
		}
	}
	if (src._epollfd > 0) {
		_epollfd = dup(src._epollfd);
		if (_epollfd <= 0) {
			if (_RSA_key)
				EVP_PKEY_free(_RSA_key);
			if (_mem)
				BIO_free(_mem);
			if (_sockfd > 0)
				close(_sockfd);
			throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not duplicate epoll fd");
		}
	}
}

Client::Client(Client&& src): _secure(src._secure), _handshake(src._handshake), _sockfd(src._sockfd), _RSA_key(src._RSA_key), _mem(src._mem), _pubkey(src._pubkey), _pubkey_len(src._pubkey_len), _epollfd(src._epollfd) {
	memcpy((void *)&_sockaddr, (void *)&src._sockaddr, sizeof(_sockaddr));
	src._sockfd = 0;
	src._RSA_key = NULL;
	src._mem = NULL;
	src._pubkey = NULL;
	src._epollfd = 0;
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
}

Client&	Client::operator=(const Client& rhs) {
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
	}
	memcpy((void *)&_sockaddr, (void *)&rhs._sockaddr, sizeof(_sockaddr));
	if (rhs._sockfd >= 0) {
		_sockfd = dup(rhs._sockfd);
		if (_sockfd <= 0) {
			if (_RSA_key)
				EVP_PKEY_free(_RSA_key);
			if (_mem)
				BIO_free(_mem);
			throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not duplicate socket");
		}
	}
	if (rhs._epollfd > 0) {
		_epollfd = dup(rhs._epollfd);
		if (_epollfd <= 0) {
			if (_RSA_key)
				EVP_PKEY_free(_RSA_key);
			if (_mem)
				BIO_free(_mem);
			if (_sockfd > 0)
				close(_sockfd);
			throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not duplicate epoll fd");
		}
	}
	return (*this);
}

Client&	Client::operator=(Client&& rhs) {
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
	return (*this);
}

void	Client::printPubkey(void)
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

bool	Client::getPacketSize(void)
{
	size_t	pos = 0;

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


int	Client::run(void) {
	std::string	buf;
	int			hasevent = 0;
	ssize_t		ret = 0;

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
		if (!(_secure && !_handshake) && buf.empty()) {
			std::getline(std::cin, buf);
			if (std::cin.eof()) {
				std::cout << "Goodbye" << std::endl;
				epoll_ctl(_epollfd, EPOLL_CTL_DEL, _sockfd, &_event);
				return 0;
			}
		}
		hasevent = epoll_wait(_epollfd, &_event, 1, 1000*60*2);
		if (hasevent > 0) {
			if (_event.events & EPOLLHUP) {
				std::cerr << "Server disconnected" << std::endl;
				return 1;
			}
			if (_secure && !_handshake) {
				if (_event.events & EPOLLOUT) {
					if (buf.empty()) {
						buf = "Length " + std::to_string(_pubkey_len);
						buf += "\n";
						buf.insert(buf.end(), _pubkey, _pubkey + _pubkey_len);
						ret = send(_sockfd, buf.data(), buf.size(), MSG_DONTWAIT);
						if (ret <= 0) {
							std::cerr << "Could not send packet to server" << std::endl;
							return 1;
						}
						buf.erase(buf.begin(), buf.begin() + ret);
					}
					else {
						ret = send(_sockfd, buf.data(), buf.size(), MSG_DONTWAIT);
						if (ret <= 0) {
							std::cerr << "Could not send packet to server" << std::endl;
							return 1;
						}
						buf.erase(buf.begin(), buf.begin() + ret);
					}
					if (buf.empty()) {
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
				buf += "\n";
				ret = send(_sockfd, buf.data(), buf.size(), MSG_DONTWAIT);
				if (ret <= 0) {
					std::cerr << "Could not send packet to server" << std::endl;
					return 1;
				}
				buf.erase(buf.begin(), buf.begin() + ret);
			}
			else {
				std::cerr << "Not implemented yet" << std::endl;
			}
		}
	}
	return 0;
}

bool	Client::decryptAESKey(void) {
    size_t  outlen;

    EVP_PKEY_CTX*   ctx = EVP_PKEY_CTX_new(this->_RSA_key, NULL);
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
    unsigned char* out_tmp_buf = NULL;
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
	return true;
}
