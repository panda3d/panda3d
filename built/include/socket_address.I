/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file socket_address.I
 * @author rdb
 * @date 2014-10-19
 */

#include "pnotify.h"

/**
 * Constructor that lets us set a port value
 */
INLINE Socket_Address::
Socket_Address(unsigned short port) {
  _addr4.sin_family = AF_INET;
  _addr4.sin_addr.s_addr = INADDR_ANY;
  _addr4.sin_port = htons(port);
}

/**
 *
 */
INLINE Socket_Address::
Socket_Address(const Socket_Address &inaddr) :
  _storage(inaddr._storage) {
}

/**
 *
 */
INLINE Socket_Address::
Socket_Address(const struct sockaddr &inaddr) {
  if (inaddr.sa_family == AF_INET) {
    _addr4 = (const sockaddr_in &)inaddr;

  } else if (inaddr.sa_family == AF_INET6) {
    _addr6 = (const sockaddr_in6 &)inaddr;

  } else {
    nassert_raise("unsupported address family");
    clear();
  }
}

/**
 *
 */
INLINE Socket_Address::
Socket_Address(const struct sockaddr_in &inaddr) :
  _addr4(inaddr) {
}

/**
 *
 */
INLINE Socket_Address::
Socket_Address(const struct sockaddr_in6 &inaddr) :
  _addr6(inaddr) {

  if (IN6_IS_ADDR_V4MAPPED(&_addr6.sin6_addr) != 0) {
    // This is really an IPv4 address disguised as an IPv6 address.
    _addr4.sin_family = AF_INET;
    _addr4.sin_addr.s_addr = ((uint32_t *)&_addr6.sin6_addr)[3];
  }
}

/**
 *
 */
INLINE Socket_Address::
Socket_Address(const struct sockaddr_storage &inaddr) :
  _storage(inaddr) {
}

/**
 * Normal Destructor
 */
INLINE Socket_Address::
~Socket_Address() {
}

/**
 *
 */
INLINE bool Socket_Address::
operator == (const Socket_Address &in) const {
  if (_storage.ss_family != in._storage.ss_family) {
    return false;
  }

  if (_storage.ss_family == AF_INET) {
    return _addr4.sin_port == in._addr4.sin_port &&
           _addr4.sin_addr.s_addr == in._addr4.sin_addr.s_addr;

  } else if (_storage.ss_family == AF_INET6) {
    return _addr6.sin6_port != in._addr6.sin6_port &&
           memcmp((char *) &_addr6.sin6_addr,
                  (char *) &in._addr6.sin6_addr,
                  sizeof(_addr6.sin6_addr)) == 0;
  }

  // Unsupported address family.
  nassert_raise("unsupported address family");
  return false;
}

/**
 *
 */
INLINE bool Socket_Address::
operator != (const Socket_Address &in) const {
  return !operator ==(in);
}

/**
 * Set to the broadcast address and a specified port
 */
INLINE bool Socket_Address::
set_broadcast(unsigned short port) {
  _addr4.sin_family = AF_INET;
  _addr4.sin_addr.s_addr = 0xffffffff;
  _addr4.sin_port = htons(port);
  return true;
}

/**
 * Set to any address and a specified port
 */
INLINE bool Socket_Address::
set_any_IP(unsigned short port) {
  _addr4.sin_family = AF_INET;
  _addr4.sin_addr.s_addr = INADDR_ANY;
  _addr4.sin_port = htons(port);
  return true;
}

/**
 * Set to any IPv6 address and a specified port.
 */
INLINE bool Socket_Address::
set_any_IPv6(unsigned short port) {
  _addr6.sin6_family = AF_INET6;
  _addr6.sin6_addr = in6addr_any;
  _addr6.sin6_port = htons(port);
  _addr6.sin6_scope_id = 0;
  return true;
}

/**
 * Set to a specified port
 */
INLINE bool Socket_Address::
set_port(unsigned short port) {
  _addr4.sin_port = htons(port);
  return true;
}

/**
 * Set the internal values to a suitable known value
 */
INLINE void Socket_Address::
clear() {
  _addr4.sin_family = AF_INET;
  _addr4.sin_addr.s_addr = INADDR_ANY;
  _addr4.sin_port = htons(0);
}

/**
 * Returns AF_INET if this is an IPv4 address, or AF_INET6 if this is an IPv6
 * address.
 */
INLINE sa_family_t Socket_Address::
get_family() const {
  return _storage.ss_family;
}

/**
 * Get the port portion as an integer
 */
INLINE unsigned short Socket_Address::
get_port() const {
  return ntohs(_addr4.sin_port);
}

/**
 *
 */
INLINE bool Socket_Address::
set_host(uint32_t in_hostname, unsigned short port) {
  memcpy(&_addr4.sin_addr, &in_hostname, sizeof(in_hostname));
  _addr4.sin_port = htons(port);
  _addr4.sin_family = AF_INET;
  return true;
}

/**
 *
 */
INLINE bool Socket_Address::
operator < (const Socket_Address &in) const {
  if (_storage.ss_family != in._storage.ss_family) {
    return _storage.ss_family < in._storage.ss_family;
  }

  if (_storage.ss_family == AF_INET) {
    if (_addr4.sin_port != in._addr4.sin_port) {
      return _addr4.sin_port < in._addr4.sin_port;
    }

    return _addr4.sin_addr.s_addr < in._addr4.sin_addr.s_addr;

  } else if (_storage.ss_family == AF_INET6) {
    if (_addr6.sin6_port != in._addr6.sin6_port) {
      return _addr6.sin6_port < in._addr6.sin6_port;
    }

    return IN6_ARE_ADDR_EQUAL(&_addr6.sin6_addr, &in._addr6.sin6_addr) != 0;
  }

  // Unsupported address family.
  nassert_raise("unsupported address family");
  return false;
}

/**
 * True if the address is zero.
 */
INLINE bool Socket_Address::
is_any() const {
  if (_storage.ss_family == AF_INET) {
    return (_addr4.sin_addr.s_addr == 0);

  } else if (_storage.ss_family == AF_INET6) {
    return IN6_IS_ADDR_UNSPECIFIED(&_addr6.sin6_addr) != 0;

  } else {
    return true;
  }
}

/**
 * True if the address is in the multicast range.
 */
INLINE bool Socket_Address::
is_mcast_range() const {
  if (_storage.ss_family == AF_INET) {
    uint32_t address = ntohl(_addr4.sin_addr.s_addr);
    // 224.0.0.0-239.255.255.255 .. e0,ef
    return (address >= 0xe0000000 && address < 0xefffffff);

  } else if (_storage.ss_family == AF_INET6) {
    // ff00::/8
    return IN6_IS_ADDR_MULTICAST(&_addr6.sin6_addr) != 0;

  } else {
    return false;
  }
}
