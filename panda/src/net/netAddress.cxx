// Filename: netAddress.cxx
// Created by:  drose (08Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "netAddress.h"
#include "config_net.h"


////////////////////////////////////////////////////////////////////
//     Function: NetAddress::Constructor
//       Access: Published
//  Description: Constructs an unspecified address.
////////////////////////////////////////////////////////////////////
NetAddress::
NetAddress() {
}

////////////////////////////////////////////////////////////////////
//     Function: NetAddress::Constructor
//       Access: Published
//  Description: Constructs an address from a given Socket_Address.
//               Normally, this constructor should not be used by user
//               code; instead, create a default NetAddress and use
//               one of the set_*() functions to set up an address.
////////////////////////////////////////////////////////////////////
NetAddress::
NetAddress(const Socket_Address &addr) : _addr(addr) {
}


////////////////////////////////////////////////////////////////////
//     Function: NetAddress::set_any
//       Access: Published
//  Description: Sets the address up to refer to a particular port,
//               but not to any particular IP.  Returns true if
//               successful, false otherwise (currently, this only
//               returns true).
////////////////////////////////////////////////////////////////////
bool NetAddress::
set_any(int port) {
  return _addr.set_any_IP(port);
}

////////////////////////////////////////////////////////////////////
//     Function: NetAddress::set_localhost
//       Access: Published
//  Description: Sets the address up to refer to a particular port,
//               on this host.
////////////////////////////////////////////////////////////////////
bool NetAddress::
set_localhost(int port) {
  return _addr.set_host("127.0.0.1", port);
}

////////////////////////////////////////////////////////////////////
//     Function: NetAddress::set_broadcast
//       Access: Published
//  Description: Sets the address to the broadcast address.
////////////////////////////////////////////////////////////////////
bool NetAddress::
set_broadcast(int port) {
  return _addr.set_broadcast(port);
}

////////////////////////////////////////////////////////////////////
//     Function: NetAddress::set_host
//       Access: Published
//  Description: Sets the address up to refer to a particular port
//               on a particular host.  Returns true if the hostname
//               is known, false otherwise.
////////////////////////////////////////////////////////////////////
bool NetAddress::
set_host(const string &hostname, int port) {
  return _addr.set_host(hostname, port);
}

////////////////////////////////////////////////////////////////////
//     Function: NetAddress::clear
//       Access: Published
//  Description: Resets the NetAddress to its initial state.
////////////////////////////////////////////////////////////////////
void NetAddress::
clear() {
  _addr.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: NetAddress::get_port
//       Access: Published
//  Description: Returns the port number to which this address refers.
////////////////////////////////////////////////////////////////////
int NetAddress::
get_port() const {
  return _addr.get_port();
}

////////////////////////////////////////////////////////////////////
//     Function: NetAddress::set_port
//       Access: Published
//  Description: Resets the port number without otherwise changing the
//               address.
////////////////////////////////////////////////////////////////////
void NetAddress::
set_port(int port) {
  _addr.set_port(port);
}

////////////////////////////////////////////////////////////////////
//     Function: NetAddress::get_ip_string
//       Access: Published
//  Description: Returns the IP address to which this address refers,
//               formatted as a string.
////////////////////////////////////////////////////////////////////
string NetAddress::
get_ip_string() const {
  return _addr.get_ip();
}

////////////////////////////////////////////////////////////////////
//     Function: NetAddress::get_ip
//       Access: Published
//  Description: Returns the IP address to which this address refers,
//               as a 32-bit integer, in host byte order.
////////////////////////////////////////////////////////////////////
PN_uint32 NetAddress::
get_ip() const {
  return _addr.GetIPAddressRaw();
}

////////////////////////////////////////////////////////////////////
//     Function: NetAddress::get_ip_component
//       Access: Published
//  Description: Returns the nth 8-bit component of the IP address.
//               An IP address has four components; component 0 is the
//               first (leftmost), and component 3 is the last
//               (rightmost) in the dotted number convention.
////////////////////////////////////////////////////////////////////
PN_uint8 NetAddress::
get_ip_component(int n) const {
  nassertr(n >= 0 && n < 4, 0);
  PN_uint32 ip_long = _addr.GetIPAddressRaw();
  const PN_uint8 *ip = (const PN_uint8 *)&ip_long;
  return ip[n];
}


////////////////////////////////////////////////////////////////////
//     Function: NetAddress::get_addr
//       Access: Published
//  Description: Returns the Socket_Address for this address.
////////////////////////////////////////////////////////////////////
const Socket_Address &NetAddress::
get_addr() const {
  return _addr;
}

////////////////////////////////////////////////////////////////////
//     Function: NetAddress::output
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void NetAddress::
output(ostream &out) const {
  out << get_ip_string();
}

//////////////////////////////////////////////////////////////
//     Function: NetAddress::get_hash
//       Access: Published
//  Description:
//////////////////////////////////////////////////////////////
size_t NetAddress::
get_hash() const {
  return  (size_t)(((int)get_ip()) ^ ((int)get_port() << 16));
}

//////////////////////////////////////////////////////////////
//     Function: NetAddress::operator ==
//       Access: Published
//  Description:
//////////////////////////////////////////////////////////////
bool NetAddress::
operator == (const NetAddress &other) const {
  return _addr == other._addr;
}

//////////////////////////////////////////////////////////////
//     Function: NetAddress::operator !=
//       Access: Published
//  Description:
//////////////////////////////////////////////////////////////
bool NetAddress::
operator != (const NetAddress &other) const {
  return _addr != other._addr;
}
