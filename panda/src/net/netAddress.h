// Filename: netAddress.h
// Created by:  drose (08Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef NETADDRESS_H
#define NETADDRESS_H

#include "pandabase.h"
#include <numeric_types.h>

#include <prio.h>

////////////////////////////////////////////////////////////////////
//       Class : NetAddress
// Description : Represents a network address to which UDP packets may
//               be sent or to which a TCP socket may be bound.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NetAddress {
PUBLISHED:
  NetAddress();
  NetAddress(const PRNetAddr &addr);

  bool set_any(int port);
  bool set_localhost(int port);
  bool set_host(const string &hostname, int port);

  void clear();

  int get_port() const;
  void set_port(int port);
  string get_ip_string() const;
  PN_uint32 get_ip() const;
  PN_uint8 get_ip_component(int n) const;

  PRNetAddr *get_addr() const;

  void output(ostream &out) const;

private:
  PRNetAddr _addr;
};

INLINE ostream &operator << (ostream &out, const NetAddress &addr) {
  addr.output(out);
  return out;
}

#endif

