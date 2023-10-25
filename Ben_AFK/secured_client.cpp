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

std::string SecuredClient::encryptRSAKey(void) {
	std::string buf = "Length " + std::to_string(_pubkey_len);
	buf += "\n";
	buf.insert(buf.end(), _pubkey, _pubkey + _pubkey_len);
	return (buf);
}

bool SecuredClient::decryptAESKey(std::string buf) {
		size_t	outlen;

		EVP_PKEY_CTX*	 ctx = EVP_PKEY_CTX_new(this->_RSA_key, nullptr);
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
		if (EVP_PKEY_decrypt(ctx, nullptr, &outlen, (const unsigned char*) buf.data(), buf.size()) <= 0) {
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
		if (EVP_PKEY_decrypt(ctx, out_tmp_buf, &outlen, (const unsigned char*) buf.data(), buf.size()) <= 0) {
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
	buf.clear();
	return true;
}

std::string SecuredClient::encrypt(std::string buf) {
	int outlen, tmplen;
	unsigned char   nextiv[16];
	size_t gcm_ivlen = 16;
	unsigned char outbuf[PIPE_BUF + 128 + 16]; //maximum allowed size if PIPE_BUF, up to 128 bytes might be added during encryption (block size) and we need to add new iv
	unsigned char outtag[16];
	OSSL_PARAM params[2] = {OSSL_PARAM_END, OSSL_PARAM_END};

	buf += '\n';
	if (buf.size() > PIPE_BUF) {
		std::cerr << "You cannot send more than " << PIPE_BUF << " bytes to Matt_daemon" << std::endl;
		return nullptr;
	}
	if (!RAND_bytes(nextiv, 16)) {
		std::cerr << "Could not generate next iv" << std::endl;
	}
	buf.insert(buf.end(), nextiv, nextiv + 16);
	params[0] = OSSL_PARAM_construct_size_t(OSSL_CIPHER_PARAM_AEAD_IVLEN, &gcm_ivlen);

	/*
	 * Initialise an encrypt operation with the cipher/mode, key, IV and
	 * IV length parameter.
	 */
	if (!EVP_EncryptInit_ex2(_ctx, _cipher, _key, _iv, params)) {
		std::cerr << "Could not initialize encryption" << std::endl;
		return nullptr;
	}

	/* Encrypt plaintext */
	if (!EVP_EncryptUpdate(_ctx, outbuf, &outlen, reinterpret_cast<unsigned char*>(buf.data()), static_cast<int>(buf.size()))) {
		std::cerr << "Could not encrypt buffer" << std::endl;
		return nullptr;
	}

	/* Finalise: note get no output for GCM */
	if (!EVP_EncryptFinal_ex(_ctx, outbuf, &tmplen)) {
		std::cerr << "Could not finalize encryption" << std::endl;
		return nullptr;
	}

	/* Get tag */
	params[0] = OSSL_PARAM_construct_octet_string(OSSL_CIPHER_PARAM_AEAD_TAG, outtag, 16);
	if (!EVP_CIPHER_CTX_get_params(_ctx, params)) {
		std::cerr << "Could not get tag" << std::endl;
		return nullptr;
	}

	/* Output tag */
	buf.assign(outbuf, outbuf + outlen);
	buf.insert(buf.end(), outtag, outtag + 16);
	memcpy(this->_iv, nextiv, 16);
	//EVP_CIPHER_CTX_reset(this->_ctx);
	return buf;
}
