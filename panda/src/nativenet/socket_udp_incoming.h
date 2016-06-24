#ifndef __SOCKET_UDP_INCOMING_H__
#define __SOCKET_UDP_INCOMING_H__

#include "pandabase.h"
#include "socket_ip.h"

/**
 * Base functionality for a UDP Reader
 */
class EXPCL_PANDA_NATIVENET Socket_UDP_Incoming : public Socket_IP {
PUBLISHED:
  inline Socket_UDP_Incoming() {}

  inline bool OpenForInput(unsigned short port);
  inline bool OpenForInput(const Socket_Address &address);
  inline bool OpenForInputMCast(const Socket_Address &address);
  inline bool GetPacket(char *data, int *max_len, Socket_Address &address);
  inline bool SendTo(const char *data, int len, const Socket_Address &address);
  inline bool InitNoAddress();
  inline bool SetToBroadCast();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Socket_IP::init_type();
    register_type(_type_handle, "Socket_UDP_Incoming",
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
 * Flips the OS bits that allow for brodcast packets to come in on this port.
 */
inline bool Socket_UDP_Incoming::
SetToBroadCast() {
  int optval = 1;

  if (setsockopt(_socket, SOL_SOCKET, SO_BROADCAST, (char *)&optval, sizeof(optval)) != 0) {
    return false;
  }
  return true;
}

/**
 * Set this socket to work without a bound external address.
 */
inline bool Socket_UDP_Incoming::
InitNoAddress() {
  Close();

  if (support_ipv6) {
    // Create a socket supporting both IPv4 and IPv6.
    _socket = DO_NEWUDP(AF_INET6);
    SetV6Only(false);
  } else {
    _socket = DO_NEWUDP(AF_INET);
  }

  return (_socket != BAD_SOCKET);
}

/**
 * Starts a UDP socket listening on a port
 */
inline bool Socket_UDP_Incoming::
OpenForInput(unsigned short port) {
  Close();

  Socket_Address address;
  if (support_ipv6) {
    // Create a socket supporting both IPv4 and IPv6.
    address.set_any_IPv6(port);
    _socket = DO_NEWUDP(AF_INET6);
    SetV6Only(false);
  } else {
    address.set_any_IP(port);
    _socket = DO_NEWUDP(AF_INET);
  }

  if (_socket == BAD_SOCKET) {
    return ErrorClose();
  }

  if (DO_BIND(_socket, &address.GetAddressInfo()) != 0) {
    return ErrorClose();
  }

  return true;
}

/**
 * Starts a UDP socket listening on a port
 */
inline bool Socket_UDP_Incoming::
OpenForInput(const Socket_Address &address) {
  Close();
  _socket = DO_NEWUDP(address.get_family());
  if (_socket == BAD_SOCKET) {
    return ErrorClose();
  }

  if (DO_BIND(_socket, &address.GetAddressInfo()) != 0) {
    return ErrorClose();
  }

  return true;
}

/**
 * Starts a UDP socket listening on a port
 */
inline bool Socket_UDP_Incoming::
OpenForInputMCast(const Socket_Address &address) {
  Close();
  _socket = DO_NEWUDP(address.get_family());
  if (_socket == BAD_SOCKET) {
    return ErrorClose();
  }

  Socket_Address wa1(address.get_port());
  if (DO_BIND(_socket, &wa1.GetAddressInfo()) != 0) {
    return ErrorClose();
  }

  int status = -1;

  const sockaddr *addr = &address.GetAddressInfo();
  if (addr->sa_family == AF_INET) {
    struct ip_mreq imreq;
    memset(&imreq, 0, sizeof(imreq));
    imreq.imr_multiaddr.s_addr = ((sockaddr_in*)addr)->sin_addr.s_addr;
    imreq.imr_interface.s_addr = INADDR_ANY; // use DEFAULT interface

    status = setsockopt(GetSocket(), IPPROTO_IP, IP_ADD_MEMBERSHIP,
                        (const char *)&imreq, sizeof(imreq));

  } else if (addr->sa_family == AF_INET6) {
    struct ipv6_mreq imreq;
    memcpy(&imreq.ipv6mr_multiaddr,
           &(((struct sockaddr_in6 *)addr)->sin6_addr),
           sizeof(struct in6_addr));
    imreq.ipv6mr_interface = 0; // use DEFAULT interface

    status = setsockopt(GetSocket(), IPPROTO_IPV6, IPV6_JOIN_GROUP,
                        (const char *)&imreq, sizeof(imreq));
  }

  return (status == 0);
}

/**
 * Grabs a dataset off the listening UDP socket and fills in the source
 * address information
 *
 */
inline bool Socket_UDP_Incoming::
GetPacket(char *data, int *max_len, Socket_Address &address) {
  int val = DO_RECV_FROM(_socket, data, *max_len, &address.GetAddressInfo());
  *max_len = 0;

  if (val <= 0) {
    if (GetLastError() != LOCAL_BLOCKING_ERROR) { // im treating a blocking error as a 0 lenght read
      return false;
    }
  } else {
    *max_len = val;
  }

  return true;
}

/**
 * Send data to specified address
 */
inline bool Socket_UDP_Incoming::
SendTo(const char *data, int len, const Socket_Address &address) {
  return (DO_SOCKET_WRITE_TO(_socket, data, len, &address.GetAddressInfo()) == len);
}

#endif //__SOCKET_UDP_INCOMING_H__
