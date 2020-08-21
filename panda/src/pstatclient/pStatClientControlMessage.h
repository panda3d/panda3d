/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatClientControlMessage.h
 * @author drose
 * @date 2000-07-09
 */

#ifndef PSTATCLIENTCONTROLMESSAGE_H
#define PSTATCLIENTCONTROLMESSAGE_H

#include "pandabase.h"

#include "pStatCollectorDef.h"

#include "pvector.h"

class Datagram;
class PStatClientVersion;

/**
 * This kind of message is sent from the client to the server on the TCP
 * socket to establish critical control information.
 */
class EXPCL_PANDA_PSTATCLIENT PStatClientControlMessage {
public:
  PStatClientControlMessage();

  void encode(Datagram &datagram) const;
  bool decode(const Datagram &datagram, PStatClientVersion *version);

  enum Type {
    T_datagram = 0,
    T_hello,
    T_define_collectors,
    T_define_threads,
    T_invalid
  };

  Type _type;

  // Used for T_hello
  std::string _client_hostname;
  std::string _client_progname;
  int _major_version;
  int _minor_version;

  // Used for T_define_collectors
  pvector<PStatCollectorDef *> _collectors;

  // Used for T_define_threads
  int _first_thread_index;
  pvector<std::string> _names;
};


#endif
