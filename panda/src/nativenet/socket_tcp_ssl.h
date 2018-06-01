#ifndef __SOCKET_TCP_SSL_H__
#define __SOCKET_TCP_SSL_H__

#include "pandabase.h"
#include "config_nativenet.h"
#include "socket_ip.h"
#include "numeric_types.h"

#ifdef HAVE_OPENSSL

#include <openssl/rsa.h>       /* SSLeay stuff */
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

extern EXPCL_PANDA_NATIVENET SSL_CTX *global_ssl_ctx;

struct SSlStartup {
  SSlStartup() {
    const SSL_METHOD *meth;
    SSLeay_add_ssl_algorithms();
    // meth = SSLv23_server_method();
    meth = SSLv23_method();
    SSL_load_error_strings();
    // I hate this cast, but older versions of OpenSSL need it.
    global_ssl_ctx = SSL_CTX_new((SSL_METHOD *) meth);
  }

  ~SSlStartup() {
    SSL_CTX_free (global_ssl_ctx);
    global_ssl_ctx = nullptr;
  }

  bool isactive() { return global_ssl_ctx != nullptr; };
};

/**
 *
 */
class EXPCL_PANDA_NATIVENET Socket_TCP_SSL : public Socket_IP {
public:
  inline Socket_TCP_SSL(SOCKET);
  inline Socket_TCP_SSL() : _ssl(nullptr) {}

  virtual inline ~Socket_TCP_SSL()
  {
      CleanSslUp();
  }

  inline int SetNoDelay();
  inline int SetLinger(int interval_seconds = 0);
  inline int DontLinger();

  inline int SetSendBufferSize(int insize);
  inline bool ActiveOpen(const Socket_Address &theaddress);
  inline int SendData(const char *data, int size);
  inline int RecvData(char *data, int size);
  inline bool ErrorIs_WouldBlocking(int err);

  inline SSL *get_ssl() { return _ssl; };

  inline void DetailErrorFormat(void);

private:
  SSL *_ssl;

  void CleanSslUp() {
    if (_ssl != nullptr) {
      SSL_shutdown(_ssl);
      SSL_free(_ssl);
      _ssl = nullptr;
    }
  }

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Socket_IP::init_type();
    register_type(_type_handle, "Socket_TCP_SSL",
                  Socket_IP::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

/**
 *
 */
inline Socket_TCP_SSL::
Socket_TCP_SSL(SOCKET sck) : ::Socket_IP(sck) {
  // right know this will only work for a accepted ie a server socket ??
  SetNonBlocking(); // maybe should be blocking?

  _ssl = SSL_new(global_ssl_ctx);
  if (_ssl == nullptr) {
    return;
  }

  SSL_set_fd(_ssl, (int)GetSocket());

  SSL_accept(_ssl);
  ERR_clear_error();

  // printf(" Ssl Accept = %d \n",err);
}

/**
 * Disable Nagle algorithm.  Don't delay send to coalesce packets
 */
inline int Socket_TCP_SSL::
SetNoDelay() {
  int nodel = 1;
  int ret1;
  ret1 = setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&nodel, sizeof(nodel));

  if (ret1 != 0) {
    return BASIC_ERROR;
  }
  return ALL_OK;
}

/**
 * will control the behavior of SO_LINGER for a TCP socket
 */
int Socket_TCP_SSL::
SetLinger(int interval_seconds) {
  linger ll;
  ll.l_linger = interval_seconds;
  ll.l_onoff = 1;
  int ret1 = setsockopt(_socket, SOL_SOCKET, SO_LINGER, (const char *)&ll, sizeof(linger));
  if (ret1 != 0) {
    return BASIC_ERROR;
  }
  return ALL_OK;
}

/**
 * Turn off the linger flag.  The socket will quickly release buffered items
 * and free up OS resources.  You may lose a stream if you use this flag and
 * do not negotiate the close at the application layer.
 */
int Socket_TCP_SSL::
DontLinger() {
  linger ll;
  ll.l_linger = 0;
  ll.l_onoff = 0;
  int ret1 = setsockopt(_socket, SOL_SOCKET, SO_LINGER, (const char *)&ll, sizeof(linger));
  if (ret1 != 0) {
    return BASIC_ERROR;
  }
  return ALL_OK;
}

/**
 * Just like it sounds.  Sets a buffered socket recv buffer size.  This
 * function does not refuse ranges outside hard-coded OS limits
 */
int Socket_TCP_SSL::
SetSendBufferSize(int insize) {
  if (setsockopt(_socket, (int) SOL_SOCKET, (int) SO_SNDBUF, (char *) &insize, sizeof(int))) {
    return BASIC_ERROR;
  }
  return ALL_OK;
}

/**
* This function will try and set the socket up for active open to a specified
* address and port provided by the input parameter
*/
bool Socket_TCP_SSL::
ActiveOpen(const Socket_Address &theaddress) {
  _socket = DO_NEWTCP(theaddress.get_family());
  if (_socket == BAD_SOCKET) {
    return false;
  }

  if (DO_CONNECT(_socket, &theaddress.GetAddressInfo()) != 0) {
    return ErrorClose();
  }

  _ssl = SSL_new(global_ssl_ctx);
  if (_ssl == nullptr) {
    return false;
  }
  SSL_set_fd(_ssl, (int)GetSocket());
  if (SSL_connect(_ssl) == -1) {
    return false;
  }
  return true;

  // return SetSslUp();
}

/**
 * Ok Lets Send the Data - if error 0 if socket closed for write or lengh is 0
 * + bytes writen ( May be smaller than requested)
 */
inline int Socket_TCP_SSL::
SendData(const char *data, int size) {
  if (_ssl == nullptr) {
    return -1;
  }

// ERR_clear_error();

  return SSL_write(_ssl, data, size);
}

/**
 * Read the data from the connection - if error 0 if socket closed for read or
 * length is 0 + bytes read ( May be smaller than requested)
 */
inline int Socket_TCP_SSL::
RecvData(char *data, int len) {
  if (_ssl == nullptr) {
    return -1;
  }

  ERR_clear_error();

  return SSL_read(_ssl, data, len);
}

/**
 * Is last error a blocking error ?? True is last error was a blocking error
 */
inline bool Socket_TCP_SSL::
ErrorIs_WouldBlocking(int err) {
  if (_ssl == nullptr || err >= 0) {
    nativenet_cat.warning()
      << "Socket_TCP_SSL::ErrorIs_WouldBlocking->Called With Error number "
      << err << " or _ssl is NULL\n";
      return false;
  }

  int ssl_error_code = SSL_get_error(_ssl,err);
  bool answer = false;

  switch(ssl_error_code) {
  case SSL_ERROR_WANT_READ:
  case SSL_ERROR_WANT_WRITE:
  case SSL_ERROR_WANT_CONNECT:
// case SSL_ERROR_WANT_ACCEPT:
    answer = true;
    break;
// hmm not sure we need this .. hmmmm
  case SSL_ERROR_SYSCALL:
    if(GETERROR() == LOCAL_BLOCKING_ERROR) {
      answer = true;
    } else {
        DetailErrorFormat();
// LOGWARNING("Socket_TCP_SSL::ErrorIs_WouldBlocking-> Not A blocking Error1
// SSl_CODe=[%d] OS=[%d]",ssl_error_code,GETERROR());
    }
    break;
  default:
    DetailErrorFormat();
// LOGWARNING("Socket_TCP_SSL::ErrorIs_WouldBlocking-> Not A blocking Error2
// SSl_CODe=[%d] OS=[%d]",ssl_error_code,GETERROR());
    answer = false;
    break;
  }

// ERR_clear_error();
  return answer;
}

inline void Socket_TCP_SSL::
DetailErrorFormat(void) {
  return; // turn on for debuging

  uint32_t l;
  char buf[256];
  char buf2[4096];
  const char *file,*data;
  int line,flags;
  uint32_t es;

  es = CRYPTO_thread_id();
  while ((l = ERR_get_error_line_data(&file, &line, &data, &flags)) != 0) {
    ERR_error_string_n(l, buf, sizeof(buf));
    BIO_snprintf(buf2, sizeof(buf2), "***%lu:%s:%s:%d:%s\n", (unsigned long) es, buf, file, line, (flags & ERR_TXT_STRING) ? data : "NoText");
    nativenet_cat.warning()
      << "Socket_TCP_SSL::DetailErrorFormat->[" << buf2 << "]\n";
  }
}

#endif  // HAVE_OPENSSL

#endif //__SOCKET_TCP_SSL_H__
