// Filename: datagram.cxx
// Created by:  drose (06Jun00)
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


#include "datagram.h"

#include "notify.h"

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
  _data.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::dump_hex
//       Access: Public
//  Description: Writes a representation of the entire datagram
//               contents, as a sequence of hex (and ASCII) values.
////////////////////////////////////////////////////////////////////
void Datagram::
dump_hex(ostream &out) const {
  const char *message = (const char *)get_data();
  size_t num_bytes = get_length();
  for (size_t line = 0; line < num_bytes; line += 16) {
    char hex[12];
    sprintf(hex, "%04x ", line);
    out << hex;

    size_t p;
    for (p = line; p < line + 16; p++) {
      if (p < num_bytes) {
        char hex[12];
        sprintf(hex, " %02x", ((unsigned int)message[p]) & 0xff);
        out << hex;
      } else {
        out << "   ";
      }
    }
    out << "  ";
    for (p = line; p < line + 16 && p < num_bytes; p++) {
      // must cast to (unsigned char) to avoid conversion to large negative integers outside of 0xFF range
      if (isgraph((unsigned char)message[p]) || message[p] == ' ') {
        out << (char)message[p];
      } else {
        out << ".";
      }
    }
    out << "\n";
  }
}


////////////////////////////////////////////////////////////////////
//     Function: Datagram::pad_bytes
//       Access: Public
//  Description: Adds the indicated number of zero bytes to the
//               datagram.
////////////////////////////////////////////////////////////////////
void Datagram::
pad_bytes(size_t size) {
  nassertv((int)size >= 0);

  if (_data == (uchar *)NULL) {
    // Create a new array.
//    _data = PTA_uchar(0);
      _data = PTA_uchar::empty_array(0);

  } else if (_data.get_ref_count() == 1) {
    // Copy on write.
    PTA_uchar new_data = PTA_uchar::empty_array(0);
    new_data.v() = _data.v();
    _data = new_data;
  }

  // Now append the data.

  // It is very important that we *don't* do this reserve() operation.
  // See the further comments in append_data(), below.
  //  _data.reserve(_data.size() + size);

  while (size > 0) {
    _data.push_back('\0');
    size--;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Datagram::append_data
//       Access: Public
//  Description: Appends some more raw data to the end of the
//               datagram.
////////////////////////////////////////////////////////////////////
void Datagram::
append_data(const void *data, size_t size) {
  nassertv((int)size >= 0);

  if (_data == (uchar *)NULL) {
    // Create a new array.
    _data = PTA_uchar::empty_array(0);

  } else if (_data.get_ref_count() != 1) {
    // Copy on write.
    PTA_uchar new_data = PTA_uchar::empty_array(0);
    new_data.v() = _data.v();
    _data = new_data;
  }

  // Now append the data.

  // It is very important that we *don't* do this reserve() operation.
  // This actually slows it down on Windows, which takes the reserve()
  // request as a fixed size the array should be set to (!) instead of
  // as a minimum size to guarantee.  This forces the array to
  // reallocate itself with *every* call to append_data!
  //  _data.reserve(_data.size() + size);

  const uchar *source = (const uchar *)data;
  for (size_t i = 0; i < size; i++) {
    _data.v().push_back(source[i]);
  }
}
