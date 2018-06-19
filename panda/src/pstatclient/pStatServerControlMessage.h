/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatServerControlMessage.h
 * @author drose
 * @date 2000-07-09
 */

#ifndef PSTATSERVERCONTROLMESSAGE_H
#define PSTATSERVERCONTROLMESSAGE_H

#include "pandabase.h"

#include "pvector.h"

class Datagram;

/**
 * This kind of message is sent from the server to the client on the TCP
 * socket to establish critical control information.
 */
class EXPCL_PANDA_PSTATCLIENT PStatServerControlMessage {
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
  std::string _server_hostname;
  std::string _server_progname;
  int _udp_port;
};


#endif
