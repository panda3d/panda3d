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
//     Function: DatagramOutputFile::open
//       Access: Public
//  Description: Opens the indicated filename for reading.  Returns
//               true if successful, false on failure.
////////////////////////////////////////////////////////////////////
bool DatagramOutputFile::
open(Filename filename) {
  close();

  // DatagramOutputFiles are always binary.
  filename.set_binary();

  _out = &_out_file;
  _owns_out = false;
  return filename.open_write(_out_file);
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramOutputFile::open
//       Access: Public
//  Description: Starts writing to the indicated stream.  Returns
//               true on success, false on failure.  The
//               DatagramOutputFile does not take ownership of the
//               stream; you are responsible for closing or deleting
//               it when you are done.
////////////////////////////////////////////////////////////////////
bool DatagramOutputFile::
open(ostream &out) {
  close();

  _out = &out;
  _owns_out = false;

  return !_out->fail();
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramOutputFile::close
//       Access: Public
//  Description: Closes the file.  This is also implicitly done when
//               the DatagramOutputFile destructs.
////////////////////////////////////////////////////////////////////
void DatagramOutputFile::
close() {
  _out_file.close();
  if (_owns_out) {
    delete _out;
  }
  _out = (ostream *)NULL;
  _owns_out = false;

  _wrote_first_datagram = false;
  _error = false;
}

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
  nassertr(_out != (ostream *)NULL, false);
  nassertr(!_wrote_first_datagram, false);

  _out->write(header.data(), header.size());
  return !_out->fail();
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramOutputFile::put_datagram
//       Access: Public, Virtual
//  Description: Writes the given datagram to the file.  Returns true
//               on success, false if there is an error.
////////////////////////////////////////////////////////////////////
bool DatagramOutputFile::
put_datagram(const Datagram &data) {
  nassertr(_out != (ostream *)NULL, false);
  _wrote_first_datagram = true;

  // First, write the size of the upcoming datagram.  We do this with
  // the help of a second datagram.
  Datagram size;
  size.add_uint32(data.get_length());
  _out->write((const char *)size.get_data(), size.get_length());

  // Now, write the datagram itself.
  _out->write((const char *)data.get_data(), data.get_length());

  return !_out->fail();
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramOutputFile::is_error
//       Access: Public, Virtual
//  Description: Returns true if the file has reached an error
//               condition.
////////////////////////////////////////////////////////////////////
bool DatagramOutputFile::
is_error() {
  if (_out == (ostream *)NULL) {
    return true;
  }

  if (_out->fail()) {
    _error = true;
  }
  return _error;
}
