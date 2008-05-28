// Filename: pStatServerControlMessage.cxx
// Created by:  drose (09Jul00)
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

#include "config_pstats.h"
#include "pStatServerControlMessage.h"

#include "datagram.h"
#include "datagramIterator.h"

////////////////////////////////////////////////////////////////////
//     Function: PStatServerControlMessage::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PStatServerControlMessage::
PStatServerControlMessage() {
  _type = T_invalid;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatServerControlMessage::encode
//       Access: Public
//  Description: Writes the message into the indicated datagram.
////////////////////////////////////////////////////////////////////
void PStatServerControlMessage::
encode(Datagram &datagram) const {
  datagram.clear();
  datagram.add_uint8(_type);
  switch (_type) {
  case T_hello:
    datagram.add_string(_server_hostname);
    datagram.add_string(_server_progname);
    datagram.add_uint16(_udp_port);
    break;

  default:
    pstats_cat.error()
      << "Invalid PStatServerControlMessage::Type " << (int)_type << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatServerControlMessage::decode
//       Access: Public
//  Description: Extracts the message from the indicated datagram.
//               Returns true on success, false on error.
////////////////////////////////////////////////////////////////////
bool PStatServerControlMessage::
decode(const Datagram &datagram) {
  DatagramIterator source(datagram);
  _type = (Type)source.get_uint8();

  switch (_type) {
  case T_hello:
    _server_hostname = source.get_string();
    _server_progname = source.get_string();
    _udp_port = source.get_uint16();
    break;

  default:
    pstats_cat.error()
      << "Read invalid PStatServerControlMessage type: " << (int)_type << "\n";
    _type = T_invalid;
    return false;
  }

  return true;
}
