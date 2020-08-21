/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file netAddress.cxx
 * @author drose
 * @date 2000-02-08
 */

#include "netAddress.h"
#include "config_net.h"


/**
 * Constructs an unspecified address.
 */
NetAddress::
NetAddress() {
}

/**
 * Constructs an address from a given Socket_Address.  Normally, this
 * constructor should not be used by user code; instead, create a default
 * NetAddress and use one of the set_*() functions to set up an address.
 */
NetAddress::
NetAddress(const Socket_Address &addr) : _addr(addr) {
}


/**
 * Sets the address up to refer to a particular port, but not to any
 * particular IP.  Returns true if successful, false otherwise (currently,
 * this only returns true).
 */
bool NetAddress::
set_any(int port) {
  return _addr.set_any_IP(port);
}

/**
 * Sets the address up to refer to a particular port, on this host.
 */
bool NetAddress::
set_localhost(int port) {
  return _addr.set_host("127.0.0.1", port);
}

/**
 * Sets the address to the broadcast address.
 */
bool NetAddress::
set_broadcast(int port) {
  return _addr.set_broadcast(port);
}

/**
 * Sets the address up to refer to a particular port on a particular host.
 * Returns true if the hostname is known, false otherwise.
 */
bool NetAddress::
set_host(const std::string &hostname, int port) {
  return _addr.set_host(hostname, port);
}

/**
 * Resets the NetAddress to its initial state.
 */
void NetAddress::
clear() {
  _addr.clear();
}

/**
 * Returns the port number to which this address refers.
 */
int NetAddress::
get_port() const {
  return _addr.get_port();
}

/**
 * Resets the port number without otherwise changing the address.
 */
void NetAddress::
set_port(int port) {
  _addr.set_port(port);
}

/**
 * Returns true if the IP address has only zeroes.
 */
bool NetAddress::
is_any() const {
  return _addr.is_any();
}

/**
 * Returns the IP address to which this address refers, formatted as a string.
 */
std::string NetAddress::
get_ip_string() const {
  return _addr.get_ip();
}

/**
 * Returns the IP address to which this address refers, as a 32-bit integer,
 * in host byte order.
 * @deprecated  Does not work with IPv6 addresses.
 */
uint32_t NetAddress::
get_ip() const {
  return _addr.GetIPAddressRaw();
}

/**
 * Returns the nth 8-bit component of the IP address.  An IP address has four
 * components; component 0 is the first (leftmost), and component 3 is the
 * last (rightmost) in the dotted number convention.
 */
uint8_t NetAddress::
get_ip_component(int n) const {
  nassertr(n >= 0 && n < 4, 0);
  uint32_t ip_long = _addr.GetIPAddressRaw();
  const uint8_t *ip = (const uint8_t *)&ip_long;
  return ip[n];
}


/**
 * Returns the Socket_Address for this address.
 */
const Socket_Address &NetAddress::
get_addr() const {
  return _addr;
}

/**
 *
 */
void NetAddress::
output(std::ostream &out) const {
  out << _addr.get_ip_port();
}

/**
 *
 */
size_t NetAddress::
get_hash() const {
  return  (size_t)(((int)get_ip()) ^ ((int)get_port() << 16));
}

/**
 *
 */
bool NetAddress::
operator == (const NetAddress &other) const {
  return _addr == other._addr;
}

/**
 *
 */
bool NetAddress::
operator != (const NetAddress &other) const {
  return _addr != other._addr;
}
