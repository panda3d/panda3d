// Filename: datagramOutputFile.cxx
// Created by:  drose (30Oct00)
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

#include "datagramOutputFile.h"

////////////////////////////////////////////////////////////////////
//     Function: DatagramOutputFile::write_header
//       Access: Public
//  Description: Writes a sequence of bytes to the beginning of the
//               datagram file.  This may be called any number of
//               times after the file has been opened and before the
//               first datagram is written.  It may not be called once
//               the first datagram is written.
////////////////////////////////////////////////////////////////////
bool DatagramOutputFile::
write_header(const string &header) {
  nassertr(!_wrote_first_datagram, false);

  _out.write(header.data(), header.size());
  return !_out.fail();
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramOutputFile::put_datagram
//       Access: Public, Virtual
//  Description: Writes the given datagram to the file.  Returns true
//               on success, false if there is an error.
////////////////////////////////////////////////////////////////////
bool DatagramOutputFile::
put_datagram(const Datagram &data) {
  _wrote_first_datagram = true;

  // First, write the size of the upcoming datagram.  We do this with
  // the help of a second datagram.
  Datagram size;
  size.add_uint32(data.get_length());
  _out.write((const char *)size.get_data(), size.get_length());

  // Now, write the datagram itself.
  _out.write((const char *)data.get_data(), data.get_length());

  return !_out.fail();
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramOutputFile::is_error
//       Access: Public, Virtual
//  Description: Returns true if the file has reached an error
//               condition.
////////////////////////////////////////////////////////////////////
bool DatagramOutputFile::
is_error() {
  if (_out.fail()) {
    _error = true;
  }
  return _error;
}
