// Filename: pStatServerControlMessage.h
// Created by:  drose (09Jul00)
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

#ifndef PSTATSERVERCONTROLMESSAGE_H
#define PSTATSERVERCONTROLMESSAGE_H

#include "pandabase.h"

#include "pvector.h"

class Datagram;

////////////////////////////////////////////////////////////////////
//       Class : PStatServerControlMessage
// Description : This kind of message is sent from the server to the
//               client on the TCP socket to establish critical
//               control information.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PStatServerControlMessage {
public:
  PStatServerControlMessage();

  void encode(Datagram &datagram) const;
  bool decode(const Datagram &datagram);

  enum Type {
    T_invalid,
    T_hello,
  };

  Type _type;

  // Used for T_hello
  string _server_hostname;
  string _server_progname;
  int _udp_port;
};


#endif

