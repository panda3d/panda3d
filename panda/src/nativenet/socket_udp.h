/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file socket_udp.h
 * @author drose
 * @date 2007-03-01
 */

#ifndef __SOCKET_UDP_H__
#define __SOCKET_UDP_H__

#include "socket_udp_incoming.h"

/**
 * Base functionality for a combination UDP Reader and Writer.  This
 * duplicates code from Socket_UDP_Outgoing, to avoid the problems of multiple
 * inheritance.
 */
class EXPCL_PANDA_NATIVENET Socket_UDP : public Socket_UDP_Incoming {
public:
PUBLISHED:
  inline Socket_UDP() {}

  // use this interface for a tagreted UDP connection
  inline bool InitToAddress(const Socket_Address &address);
public:
  inline bool Send(const char *data, int len);
PUBLISHED:
  inline bool Send(const std::string &data);
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
    Socket_UDP_Incoming::init_type();
    register_type(_type_handle, "Socket_UDP",
                  Socket_UDP_Incoming::get_class_type());
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
inline bool Socket_UDP::
SetToBroadCast() {
  int optval = 1;

  if (setsockopt(_socket, SOL_SOCKET, SO_BROADCAST, (char *)&optval, sizeof(optval)) != 0) {
    return false;
  }
  return true;
}

/**
 * Connects the socket to a Specified address
 */
inline bool Socket_UDP::
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
 * Send data to connected address
 */
inline bool Socket_UDP::
Send(const char *data, int len) {
  return (DO_SOCKET_WRITE(_socket, data, len) == len);
}

/**
 * Send data to connected address
 */
inline bool Socket_UDP::
Send(const std::string &data) {
  return Send(data.data(), data.size());
}

/**
 * Send data to specified address
 */
inline bool Socket_UDP::
SendTo(const char *data, int len, const Socket_Address &address) {
  return (DO_SOCKET_WRITE_TO(_socket, data, len, &address.GetAddressInfo()) == len);
}

/**
 * Send data to specified address
 */
inline bool Socket_UDP::
SendTo(const std::string &data, const Socket_Address &address) {
  return SendTo(data.data(), data.size(), address);
}

#endif //__SOCKET_UDP_H__
