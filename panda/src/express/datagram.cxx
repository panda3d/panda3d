// Filename: datagram.cxx
// Created by:  drose (06Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "datagram.h"

#include <notify.h>

#include <stdio.h>  // for sprintf().

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

