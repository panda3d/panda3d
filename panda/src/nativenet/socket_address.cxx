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
 * @date 2016-06-17
 */

#include "socket_address.h"
#include "config_downloader.h"

/**
 * This function will take a port and string-based TCP address and initialize
 * the address with this information.  Returns true on success; on failure, it
 * returns false and the address may be undefined.
 */
bool Socket_Address::
set_host(const std::string &hostname, unsigned short port) {
  // hmm inet_addr4 does not resolve 255.255.255.255 on ME98 ?? * HACK * ??
  if (hostname == "255.255.255.255") {
    return set_broadcast(port);
  }

  struct addrinfo hints, *res = nullptr;
  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_ADDRCONFIG;
  hints.ai_family = support_ipv6 ? AF_UNSPEC : AF_INET;

  if (getaddrinfo(hostname.c_str(), nullptr, &hints, &res)) {
    return false;
  }

  nassertr(res->ai_addrlen <= sizeof(_storage), false);
  memcpy(&_storage, res->ai_addr, res->ai_addrlen);
  freeaddrinfo(res);

  _addr4.sin_port = htons(port);
  return true;
}

/**
 * Initializes the address from a string specifying both the address and port,
 * separated by a colon.  An IPv6 address must be enclosed in brackets.
 */
bool Socket_Address::
set_host(const std::string &hostname) {
  std::string::size_type pos = hostname.rfind(':');
  if (pos == std::string::npos) {
    return false;
  }

  std::string::size_type host_begin = 0;
  std::string::size_type host_end = pos;

  // Strip spaces.
  while (host_begin < host_end && isspace(hostname[host_begin])) {
    ++host_begin;
  }

  while (host_begin < host_end && isspace(hostname[host_end - 1])) {
    --host_end;
  }

  if (host_begin < host_end && hostname[host_begin] == '[') {
    // Looks like an IPv6 address; extract from the brackets.
    host_begin += 1;
    if (hostname[host_end - 1] == ']') {
      host_end -= 1;
    } else {
      return false;
    }
  }

  std::string host = hostname.substr(host_begin, host_end - host_begin);
  std::string port = hostname.substr(pos + 1, 100);

  unsigned short port_dig = (unsigned short)atoi(port.c_str());
  return set_host(host, port_dig);
}

/**
 * Return the IP address portion in dot notation string.
 */
std::string Socket_Address::
get_ip() const {
  char buf[48];
  buf[0] = 0;

  if (_storage.ss_family == AF_INET) {
    getnameinfo(&_addr, sizeof(sockaddr_in), buf, sizeof(buf), nullptr, 0, NI_NUMERICHOST);

  } else if (_storage.ss_family == AF_INET6) {
    getnameinfo(&_addr, sizeof(sockaddr_in6), buf, sizeof(buf), nullptr, 0, NI_NUMERICHOST);

  } else {
    nassert_raise("unsupported address family");
  }

  return std::string(buf);
}

/**
 * Return the ip address/port in dot notation string.  If this is an IPv6
 * address, it will be enclosed in square brackets.
 */
std::string Socket_Address::
get_ip_port() const {
  char buf[100];  // 100 is more than enough for any ip address:port combo..
  buf[0] = 0;

  if (_storage.ss_family == AF_INET) {
    getnameinfo(&_addr, sizeof(sockaddr_in), buf, sizeof(buf), nullptr, 0, NI_NUMERICHOST);
    sprintf(buf + strlen(buf), ":%hu", get_port());

  } else if (_storage.ss_family == AF_INET6) {
    // Protect the IPv6 address within square brackets.
    buf[0] = '[';
    getnameinfo(&_addr, sizeof(sockaddr_in6), buf + 1, sizeof(buf) - 1, nullptr, 0, NI_NUMERICHOST);
    sprintf(buf + strlen(buf), "]:%hu", get_port());

  } else {
    nassert_raise("unsupported address family");
  }

  return std::string(buf);
}

/**
 * Returns a raw 32-bit unsigned integer representing the IPv4 address.
 * @deprecated  Does not work with IPv6 addresses.
 */
unsigned long Socket_Address::
GetIPAddressRaw() const {
  if (_addr.sa_family == AF_INET) {
    return _addr4.sin_addr.s_addr;
  }
  if (_addr.sa_family == AF_INET6) {
    // Okay, if we got here, something probably went wrong, but let's see if
    // we can offer a meaningful translation for mapped addresses.
    uint32_t *parts = (uint32_t *)&_addr6.sin6_addr;
    if (parts[0] == 0 && parts[1] == 0 && (parts[2] == 0 || parts[2] == htonl(0xffff))) {
      if (parts[2] == 0 && parts[3] == htonl(1)) {
        // Special exception for localhost.
        return 0x1000007f;
      } else {
        return parts[3];
      }
    }
  }
  nassert_raise("GetIPAddressRaw() can only be called on an IPv4 address");
  return 0;
}
