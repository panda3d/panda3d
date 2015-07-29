
#ifndef SSL_H
#define SSL_H

struct BIO;
struct SSL_CTX;
struct EVP_CIPHER_CTX;
struct EVP_PKEY;
struct X509;
struct X509_STORE;
struct X509_NAME;
struct SSL;
#define STACK_OF(type) struct stack_st_##type

#endif
