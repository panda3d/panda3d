#ifndef __SOCKET_UDP_OUTGOING_H__
#define __SOCKET_UDP_OUTGOING_H__

#include "config_nativenet.h"
#include "socket_ip.h"

/**
 * Base functionality for a UDP sending socket
 */
class EXPCL_PANDA_NATIVENET Socket_UDP_Outgoing : public Socket_IP {
PUBLISHED:
  inline Socket_UDP_Outgoing() {}

  // use this interface for a tagreted UDP connection
  inline bool InitToAddress(const Socket_Address &address);
public:
  inline bool Send(const char *data, int len);
PUBLISHED:
  inline bool Send(const std::string &data);
  // use this interface for a none tagreted UDP connection
  inline bool InitNoAddress();
public:
  inline bool SendTo(const char *data, int len, const Socket_Address &address);
PUBLISHED:
  inline bool SendTo(const std::string &data, const Socket_Address &address);
  inline bool SetToBroadCast();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Socket_IP::init_type();
    register_type(_type_handle, "Socket_UDP_Outgoing",
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
 * Ask the OS to let us receive broadcast packets on this port.
 */
inline bool Socket_UDP_Outgoing::
SetToBroadCast() {
  int optval = 1;

  if (setsockopt(_socket, SOL_SOCKET, SO_BROADCAST, (char *)&optval, sizeof(optval)) != 0) {
    return false;
  }
  return true;
}

/**
 * Connects the Socket to a specified address
 */
inline bool Socket_UDP_Outgoing::
InitToAddress(const Socket_Address &address) {
  if (InitNoAddress() != true) {
    return false;
  }

  if (DO_CONNECT(_socket, &address.GetAddressInfo()) != 0) {
    return ErrorClose();
  }

  return true;
}

/**
 * This will set a udp up for targeted sends.
 */
inline bool Socket_UDP_Outgoing::
InitNoAddress() {
  Close();
  _socket = DO_NEWUDP(AF_INET);
  if (_socket == BAD_SOCKET) {
    return false;
  }

  return true;
}

/**
 * Send data to connected address
 */
inline bool Socket_UDP_Outgoing::
Send(const char *data, int len) {
  return (DO_SOCKET_WRITE(_socket, data, len) == len);
}

/**
 * Send data to connected address
 */
inline bool Socket_UDP_Outgoing::
Send(const std::string &data) {
  return Send(data.data(), data.size());
}

/**
 * Send data to specified address
 */
inline bool Socket_UDP_Outgoing::
SendTo(const char *data, int len, const Socket_Address &address) {
  return (DO_SOCKET_WRITE_TO(_socket, data, len, &address.GetAddressInfo()) == len);
}

/**
 * Send data to specified address
 */
inline bool Socket_UDP_Outgoing::
SendTo(const std::string &data, const Socket_Address &address) {
  return SendTo(data.data(), data.size(), address);
}

#endif //__SOCKET_UDP_OUTGOING_H__
