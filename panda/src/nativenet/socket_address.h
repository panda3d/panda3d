/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file socket_address.h
 * @author rdb
 * @date 2014-10-19
 */

#ifndef SOCKET_ADDRESS_H
#define SOCKET_ADDRESS_H

#include "pandabase.h"
#include "numeric_types.h"
#include "socket_portable.h"

/**
 * A simple place to store and manipulate tcp and port address for
 * communication layer
 */
class EXPCL_PANDA_NATIVENET Socket_Address {
public:
  INLINE Socket_Address(const struct sockaddr &inaddr);
  INLINE Socket_Address(const struct sockaddr_in &inaddr);
  INLINE Socket_Address(const struct sockaddr_in6 &inaddr);
  INLINE Socket_Address(const struct sockaddr_storage &inaddr);
  INLINE struct sockaddr &GetAddressInfo() { return _addr; }
  INLINE const struct sockaddr &GetAddressInfo() const { return _addr; }

PUBLISHED:
  INLINE explicit Socket_Address(unsigned short port = 0);
  INLINE Socket_Address(const Socket_Address &inaddr);

  INLINE virtual ~Socket_Address();

  INLINE bool set_any_IP(unsigned short port);
  INLINE bool set_any_IPv6(unsigned short port);
  INLINE bool set_port(unsigned short port);
  INLINE bool set_broadcast(unsigned short port);

  bool set_host(const std::string &hostname, unsigned short port);
  bool set_host(const std::string &hostname);
  INLINE bool set_host(uint32_t ip4addr, unsigned short port);
  INLINE void clear();

  INLINE sa_family_t get_family() const;
  INLINE unsigned short get_port() const;
  std::string get_ip() const ;
  std::string get_ip_port() const;
  unsigned long GetIPAddressRaw() const;

  INLINE bool operator ==(const Socket_Address &in) const;
  INLINE bool operator !=(const Socket_Address &in) const;
  INLINE bool operator < (const Socket_Address &in) const;

  INLINE bool is_any() const;
  INLINE bool is_mcast_range() const;

private:
  union {
    sockaddr _addr;
    sockaddr_in _addr4;
    sockaddr_in6 _addr6;
    sockaddr_storage _storage;
  };
};

#include "socket_address.I"

#endif  // SOCKET_ADDRESS_H
