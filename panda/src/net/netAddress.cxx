// Filename: netAddress.cxx
// Created by:  drose (08Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "netAddress.h"
#include "pprerror.h"
#include "config_net.h"

#include <prio.h>
#include <prnetdb.h>


////////////////////////////////////////////////////////////////////
//     Function: NetAddress::Constructor
//       Access: Public
//  Description: Constructs an unspecified address.
////////////////////////////////////////////////////////////////////
NetAddress::
NetAddress() {
  PR_InitializeNetAddr(PR_IpAddrLoopback, 0, &_addr);
}

////////////////////////////////////////////////////////////////////
//     Function: NetAddress::Constructor
//       Access: Public
//  Description: Constructs an address from a given PRNetAddr.
//               Normally, this constructor should not be used by user
//               code; instead, create a default NetAddress and use
//               one of the set_*() functions to set up an address.
////////////////////////////////////////////////////////////////////
NetAddress::
NetAddress(const PRNetAddr &addr) : _addr(addr) {
}


////////////////////////////////////////////////////////////////////
//     Function: NetAddress::set_any
//       Access: Public
//  Description: Sets the address up to refer to a particular port,
//               but not to any particular IP.  Returns true if
//               successful, false otherwise (currently, this only
//               returns true).
////////////////////////////////////////////////////////////////////
bool NetAddress::
set_any(int port) {
  PR_InitializeNetAddr(PR_IpAddrAny, port, &_addr);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NetAddress::set_localhost
//       Access: Public
//  Description: Sets the address up to refer to a particular port,
//               on this host.
////////////////////////////////////////////////////////////////////
bool NetAddress::
set_localhost(int port) {
  PR_InitializeNetAddr(PR_IpAddrLoopback, port, &_addr);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NetAddress::set_host
//       Access: Public
//  Description: Sets the address up to refer to a particular port
//               on a particular host.  Returns true if the hostname
//               is known, false otherwise.
////////////////////////////////////////////////////////////////////
bool NetAddress::
set_host(const string &hostname, int port) {
  // If the hostname appears to be a dot-separated IPv4 address, then
  // parse it directly and store it.  Some OS system libraries
  // (notably Win95) can't parse this themselves.
  union {
    PRUint32 l;
    unsigned char n[4];
  } ipaddr;
  int ni = 0;
  bool is_ip = true;
  size_t p = 0;
  size_t q = 0;
  unsigned int num = 0;

  while (p < hostname.length() && ni < 4 && is_ip) {
    if (hostname[p] == '.' && p > q) {
      // Now we have a number between q and p.
      ipaddr.n[ni] = (unsigned char)num;
      p++;
      q = p;
      num = 0;
      ni++;

      if (num >= 256 || ni >= 4) {
        is_ip = false;
      }

    } else if (isdigit(hostname[p])) {
      num = 10 * num + (unsigned int)(hostname[p] - '0');
      p++;
      if (num >= 256) {
        is_ip = false;
      }
    } else {
      is_ip = false;
    }
  }

  if (p == hostname.length() && ni < 4 && is_ip && p > q) {
    ipaddr.n[ni] = (unsigned char)num;
    ni++;

    if (num >= 256) {
      is_ip = false;
    }
  }

  if (p == hostname.length() && ni == 4 && is_ip) {
    net_cat.debug()
      << "Parsed IP " << (int)ipaddr.n[0] << "." << (int)ipaddr.n[1]
      << "." << (int)ipaddr.n[2] << "." << (int)ipaddr.n[3] << "\n";

    memset(&_addr, 0, sizeof(PRNetAddr));
    _addr.inet.family = PR_AF_INET;
    _addr.inet.port = PR_htons(port);
    _addr.inet.ip = ipaddr.l;

  } else {
    // If it's not a numeric IPv4 address, pass the whole thing on to
    // GetHostByName and let NSPR deal with it.

    char buf[PR_NETDB_BUF_SIZE];
    PRHostEnt host;
    PRStatus result =
      PR_GetHostByName(hostname.c_str(), buf, PR_NETDB_BUF_SIZE, &host);
    if (result != PR_SUCCESS) {
      pprerror("PR_GetHostByName");
      net_cat.error()
        << "Unable to look up hostname " << hostname << ".\n";
      return false;
    }

    PRIntn next = PR_EnumerateHostEnt(0, &host, port, &_addr);

    if (next == -1) {
      pprerror("PR_EnumerateHostEnt");
      return false;
    } else if (next == 0) {
      net_cat.error()
        << "No addresses available for " << hostname << ".\n";
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NetAddress::clear
//       Access: Public
//  Description: Resets the NetAddress to its initial state.
////////////////////////////////////////////////////////////////////
void NetAddress::
clear() {
  PR_InitializeNetAddr(PR_IpAddrLoopback, 0, &_addr);
}

////////////////////////////////////////////////////////////////////
//     Function: NetAddress::get_port
//       Access: Public
//  Description: Returns the port number to which this address refers.
////////////////////////////////////////////////////////////////////
int NetAddress::
get_port() const {
  return PR_ntohs(_addr.inet.port);
}

////////////////////////////////////////////////////////////////////
//     Function: NetAddress::set_port
//       Access: Public
//  Description: Resets the port number without otherwise changing the
//               address.
////////////////////////////////////////////////////////////////////
void NetAddress::
set_port(int port) {
  PR_InitializeNetAddr(PR_IpAddrNull, port, &_addr);
}

////////////////////////////////////////////////////////////////////
//     Function: NetAddress::get_ip_string
//       Access: Public
//  Description: Returns the IP address to which this address refers,
//               formatted as a string.
////////////////////////////////////////////////////////////////////
string NetAddress::
get_ip_string() const {
  static const int buf_len = 1024;
  char buf[buf_len];

  PRStatus result =
    PR_NetAddrToString(&_addr, buf, buf_len);
  if (result != PR_SUCCESS) {
    pprerror("PR_NetAddrToString");
    return "error";
  }

  return string(buf);
}

////////////////////////////////////////////////////////////////////
//     Function: NetAddress::get_ip
//       Access: Public
//  Description: Returns the IP address to which this address refers,
//               as a 32-bit integer, in host byte order.
////////////////////////////////////////////////////////////////////
PN_uint32 NetAddress::
get_ip() const {
  return PR_ntohl(_addr.inet.ip);
}

////////////////////////////////////////////////////////////////////
//     Function: NetAddress::get_ip_component
//       Access: Public
//  Description: Returns the nth 8-bit component of the IP address.
//               An IP address has four components; component 0 is the
//               first (leftmost), and component 3 is the last
//               (rightmost) in the dotted number convention.
////////////////////////////////////////////////////////////////////
PN_uint8 NetAddress::
get_ip_component(int n) const {
  nassertr(n >= 0 && n < 4, 0);
  const PN_uint8 *ip = (const PN_uint8 *)&_addr.inet.ip;
  return ip[n];
}


////////////////////////////////////////////////////////////////////
//     Function: NetAddress::get_addr
//       Access: Public
//  Description: Returns the PRNetAddr for this address.
////////////////////////////////////////////////////////////////////
PRNetAddr *NetAddress::
get_addr() const {
  return (PRNetAddr *)&_addr;
}

////////////////////////////////////////////////////////////////////
//     Function: NetAddress::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void NetAddress::
output(ostream &out) const {
  out << get_ip_string();
}
