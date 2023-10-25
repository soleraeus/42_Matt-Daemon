#ifndef SECURED_CLIENT_HPP
# define SECURED_CLIENT_HPP

# include <arpa/inet.h>
# include <iostream>
# include <iomanip>
# include <sys/socket.h>
# include <unistd.h>
# include <string.h>
# include <system_error>
# include <sys/epoll.h>
# include <sys/types.h>

//Openssl
# include <openssl/bio.h>
# include <openssl/core_names.h>
# include <openssl/evp.h>
# include <openssl/pem.h>
# include <openssl/rand.h>
# include <openssl/rsa.h>

class	SecuredClient {
	public:
		explicit SecuredClient(void);
		explicit SecuredClient(const SecuredClient &src);
		explicit SecuredClient(SecuredClient&& src);

		~SecuredClient(void);

		SecuredClient& operator=(const SecuredClient& rhs);
		SecuredClient& operator=(SecuredClient&& rhs);

	private:
		EVP_PKEY		*_RSA_key;
		BIO				*_mem;
		char			*_pubkey;
		long			_pubkey_len;
		unsigned char	_key[32];
		unsigned char	_iv[16];
		EVP_CIPHER_CTX	*_ctx;
		EVP_CIPHER		*_cipher;
};

#endif
