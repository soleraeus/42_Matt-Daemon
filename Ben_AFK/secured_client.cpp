#include "secured_client.hpp"

SecuredClient::SecuredClient(void): _RSA_key(nullptr), _mem(nullptr), _pubkey(nullptr), _pubkey_len(0), _ctx(nullptr), _cipher(nullptr) {
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

SecuredClient::SecuredClient(const SecuredClient &src):  _RSA_key(nullptr), _mem(nullptr), _pubkey(nullptr), _pubkey_len(0), _ctx(nullptr), _cipher(nullptr) {
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
}

SecuredClient::SecuredClient(SecuredClient&& src): _RSA_key(src._RSA_key), _mem(src._mem), _pubkey(src._pubkey), _pubkey_len(src._pubkey_len), _ctx(src._ctx), _cipher(src._cipher) {
	src._RSA_key = nullptr;
	src._mem = nullptr;
	src._pubkey = nullptr;
	src._ctx = nullptr;
	src._cipher = nullptr;
}

SecuredClient::~SecuredClient(void) {
	if (_RSA_key)
		EVP_PKEY_free(_RSA_key);
	if (_mem)
		BIO_free(_mem);
	if (_ctx)
		EVP_CIPHER_CTX_free(_ctx);
	if (_cipher)
		EVP_CIPHER_free(_cipher);
}

SecuredClient & SecuredClient::operator=(const SecuredClient& rhs) {
	if (this == &rhs)
		return (*this);
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
	return (*this);
}

SecuredClient & SecuredClient::operator=(SecuredClient&& rhs) {
	if (this == &rhs)
		return (*this);
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
	_RSA_key = rhs._RSA_key;
	rhs._RSA_key = nullptr;
	_mem = rhs._mem;
	rhs._mem = nullptr;
	_pubkey = rhs._pubkey;
	rhs._pubkey = nullptr;
	_pubkey_len = rhs._pubkey_len;
	_ctx = rhs._ctx;
	rhs._ctx = nullptr;
	_cipher = rhs._cipher;
	rhs._cipher = nullptr;
	return (*this);
}
