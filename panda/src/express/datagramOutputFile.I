// Filename: datagramOutputFile.I
// Created by:  drose (27Oct00)
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


////////////////////////////////////////////////////////////////////
//     Function: DatagramOutputFile::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DatagramOutputFile::
DatagramOutputFile() {
  _error = true;
  _wrote_first_datagram = false;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramOutputFile::open
//       Access: Public
//  Description: Opens the indicated filename for reading.  Returns
//               true if successful, false on failure.
////////////////////////////////////////////////////////////////////
INLINE bool DatagramOutputFile::
open(Filename filename) {
  // DatagramOutputFiles are always binary.
  _wrote_first_datagram = false;
  _error = false;
  filename.set_binary();
  return filename.open_write(_out);
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramOutputFile::close
//       Access: Public
//  Description: Closes the file.  This is also implicitly done when
//               the DatagramOutputFile destructs.
////////////////////////////////////////////////////////////////////
INLINE void DatagramOutputFile::
close() {
  _out.close();
}
