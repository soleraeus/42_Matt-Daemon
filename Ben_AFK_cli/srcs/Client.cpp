/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bdetune <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/19 20:55:26 by bdetune           #+#    #+#             */
/*   Updated: 2023/10/19 22:37:32 by bdetune          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(const char* ip, bool secure): _secure(secure), _sockfd(0), _RSA_key(NULL), _mem(NULL), _pubkey(NULL), _pubkey_len(0) {
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
	if ((_sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0)) <= 0) {
		EVP_PKEY_free(_RSA_key);
		BIO_free(_mem);
		throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not create socket");
	}
	_sockaddr.sin_family = AF_INET;
    _sockaddr.sin_port = secure ? htons(4343) : htons(4242);
	if (inet_pton(AF_INET, ip ? ip : "127.0.0.1", &_sockaddr.sin_addr) <= 0) {
		EVP_PKEY_free(_RSA_key);
		BIO_free(_mem);
		close(_sockfd);
		throw std::system_error(std::make_error_code(std::errc::bad_address), "Bad ip address provided");
	}
}

Client::Client(const Client& src): _secure(src._secure), _sockfd(0), _RSA_key(NULL), _mem(NULL), _pubkey(NULL), _pubkey_len(0) {
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
			throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not duplicate socket");
		}
	}
}

Client::Client(Client&& src): _secure(src._secure), _sockfd(src._sockfd), _RSA_key(src._RSA_key), _mem(src._mem), _pubkey(src._pubkey), _pubkey_len(src._pubkey_len) {
	memcpy((void *)&_sockaddr, (void *)&src._sockaddr, sizeof(_sockaddr));
	src._sockfd = 0;
	src._RSA_key = NULL;
	src._mem = NULL;
	src._pubkey = NULL;
}

Client::~Client(void) {
	if (_sockfd > 0)
		close(_sockfd);
	if (_RSA_key)
		EVP_PKEY_free(_RSA_key);
	if (_mem)
		BIO_free(_mem);
}

Client&	Client::operator=(const Client& rhs) {
	if (this == &rhs)
		return (*this);
	_secure = rhs._secure;
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
			throw std::system_error(std::make_error_code(std::errc::operation_canceled), "Could not duplicate socket");
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
