// Filename: datagram.cxx
// Created by:  drose (06Jun00)
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


#include "datagram.h"

#include <notify.h>

// for sprintf().
#include <stdio.h>  

TypeHandle Datagram::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: Datagram::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
Datagram::
~Datagram() {
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::clear
//       Access: Public, Virtual
//  Description: Resets the datagram to empty, in preparation for
//               building up a new datagram.
////////////////////////////////////////////////////////////////////
void Datagram::
clear() {
  _message = string();
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::dump_hex
//       Access: Public
//  Description: Writes a representation of the entire datagram
//               contents, as a sequence of hex (and ASCII) values.
////////////////////////////////////////////////////////////////////
void Datagram::
dump_hex(ostream &out) const {
  for (size_t line = 0; line < _message.length(); line += 16) {
    char hex[12];
    sprintf(hex, "%04x ", line);
    out << hex;

    size_t p;
    for (p = line; p < line + 16; p++) {
      if (p < _message.length()) {
        char hex[12];
        sprintf(hex, " %02x", ((unsigned int)_message[p]) & 0xff);
        out << hex;
      } else {
        out << "   ";
      }
    }
    out << "  ";
    for (p = line; p < line + 16 && p < _message.length(); p++) {
      if (isgraph(_message[p]) || _message[p] == ' ') {
        out << (char)_message[p];
      } else {
        out << ".";
      }
    }
    out << "\n";
  }
}

