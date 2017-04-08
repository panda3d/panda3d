#ifndef __SOCKET_IP_H__
#define __SOCKET_IP_H__

#include "pandabase.h"
#include "socket_portable.h"
#include "socket_address.h"
#include "typedObject.h"
#include "config_downloader.h"

// forward declarations for friends...
class Socket_TCP;
class Socket_UDP;
class Socket_TCP_Listen;
class Socket_UDP_Incoming;
class Socket_UDP_Outgoing;

/**
 * Base functionality for a INET domain Socket This call should be the
 * starting point for all other unix domain sockets.
 *
 * SocketIP |
 * ------------------------------------------------------------------- |
 * |                       |                           | SocketTCP
 * SocketTCP_Listen    SocketUDP_Incoming   SocketUDP_OutBound
 *
 */
class EXPCL_PANDA_NATIVENET Socket_IP : public TypedObject {
public:
PUBLISHED:
  inline Socket_IP();
  inline Socket_IP(SOCKET in);
  virtual ~Socket_IP();

  inline void Close();
  inline static int GetLastError();
  inline int SetNonBlocking();
  inline int SetBlocking();
  inline bool SetReuseAddress(bool flag = true);
  inline bool SetV6Only(bool flag);
  inline bool Active();
  inline int SetRecvBufferSize(int size);
  inline void SetSocket(SOCKET ins);
  inline SOCKET GetSocket();
  inline SOCKET GetSocket() const;
  inline Socket_Address GetPeerName(void) const;

  inline static int InitNetworkDriver() { return init_network(); };

private:
  inline bool ErrorClose();

  SOCKET _socket;   // see socket_portable.h

  friend class Socket_TCP;
  friend class Socket_UDP;
  friend class Socket_TCP_Listen;
  friend class Socket_UDP_Incoming;
  friend class Socket_UDP_Outgoing;
  friend class Socket_TCP_SSL;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "Socket_IP",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

/**
 * Used by internal to force a close.  Returns false.
 */
inline bool Socket_IP::
ErrorClose() {
  if (Active()) {
    DO_CLOSE(_socket);
  }

  _socket = BAD_SOCKET;
  return false;
}

/**
 * Ask if the socket is open (allocated)
 */
inline bool Socket_IP::
Active() {
  return (_socket != BAD_SOCKET);
}

/**
 * Def Constructor
 */
inline Socket_IP::
Socket_IP() {
  _socket = BAD_SOCKET;
}

/**
 * Assigns an existing socket to this class
 */
inline Socket_IP::
Socket_IP(SOCKET ins) {
  _socket = ins;
}

/**
 * Destructor
 */
inline Socket_IP::
~Socket_IP() {
  Close();
}

/**
 * Closes a socket if it is open (allocated).
 */
inline void Socket_IP::
Close() {
  if (Active()) {
    DO_CLOSE(_socket);
  }

  _socket = BAD_SOCKET;
}

/**
 * Gets the last errcode from a socket operation.
 */
inline int Socket_IP::
GetLastError() {
  return GETERROR();
}

/**
 * Assigns an existing socket to this class
 */
inline void Socket_IP::
SetSocket(SOCKET ins) {
  Close();
  _socket = ins;
}

/**
 * Ok it sets the recv buffer size for both tcp and UDP
 */
int Socket_IP::
SetRecvBufferSize(int insize) {
  if (setsockopt(_socket, (int) SOL_SOCKET, (int) SO_RCVBUF, (char *) &insize, sizeof(int))) {
    return BASIC_ERROR;
  }

  return ALL_OK;
}

/**
 * this function will throw a socket into non-blocking mode
 */
inline int Socket_IP::
SetNonBlocking() {
#ifdef BSDBLOCK
  int flags = fcntl(_socket, F_GETFL, 0);
  flags = flags | O_NONBLOCK;
  fcntl(_socket, F_SETFL, flags);
  return ALL_OK;
#else
  unsigned long  val = LOCAL_NONBLOCK;
  unsigned lanswer = 0;
  lanswer = SOCKIOCTL(_socket, LOCAL_FL_SET, &val);
  if (lanswer != 0) {
    return BASIC_ERROR;
  }
  return ALL_OK;
#endif
}

/**
 * Set the socket to block on subsequent calls to socket functions that
 * address this socket
 */
inline int Socket_IP::
SetBlocking() {
#ifdef BSDBLOCK
  int flags = fcntl(_socket, F_GETFL, 0);
  flags &= ~O_NONBLOCK;
  fcntl(_socket, F_SETFL, flags);
  return ALL_OK;
#else
  unsigned long val = 0;
  unsigned lanswer = 0;
  lanswer = SOCKIOCTL(_socket, LOCAL_FL_SET, &val);
  if (lanswer != 0) {
    return BASIC_ERROR;
  }
  return ALL_OK;
#endif
}

/**
 * Informs a socket to reuse IP address as needed
 */
inline bool Socket_IP::
SetReuseAddress(bool flag) {
  int value = flag;
  if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&value, sizeof(value)) != 0) {
    return false;
  }
  return true;
}

/**
 * Sets a flag indicating whether this IPv6 socket should operate in
 * dual-stack mode or not.
 */
inline bool Socket_IP::
SetV6Only(bool flag) {
  int value = flag ? 1 : 0;
  if (setsockopt(_socket, IPPROTO_IPV6, IPV6_V6ONLY, (const char *)&value, sizeof(value))) {
    return false;
  }
  return true;
}

/**
 * Gets the base socket type
 */
inline SOCKET Socket_IP::
GetSocket() {
  return _socket;
}

/**
 * Get The RAW file id of the socket
 */
inline SOCKET Socket_IP::
GetSocket() const {
  return _socket;
}

/**
 * Wrapper on berkly getpeername...
 */
inline Socket_Address Socket_IP::
GetPeerName(void) const {
  sockaddr_storage name;
  socklen_t name_len = sizeof(name);
  memset(&name, 0, name_len);

  getpeername(_socket, (sockaddr *)&name, &name_len);
  return Socket_Address(name);
}

#endif //__SOCKET_IP_H__
