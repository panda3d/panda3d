
#ifndef SSL_H
#define SSL_H

typedef struct bio_st BIO;
typedef struct ssl_ctx_st SSL_CTX;
typedef struct evp_cipher_ctx_st EVP_CIPHER_CTX;
typedef struct evp_pkey_st EVP_PKEY;
typedef struct x509_st X509;
typedef struct x509_store_st X509_STORE;
typedef struct X509_name_st X509_NAME;
typedef struct ssl_cipher_st SSL_CIPHER;
struct SSL;
#define STACK_OF(type) struct stack_st_##type

#endif
