// Filename: datagramInputFile.I
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


////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DatagramInputFile::
DatagramInputFile() {
  _error = true;
  _read_first_datagram = false;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::open
//       Access: Public
//  Description: Opens the indicated filename for reading.  Returns
//               true if successful, false on failure.
////////////////////////////////////////////////////////////////////
INLINE bool DatagramInputFile::
open(Filename filename) {
  // DatagramInputFiles are always binary.
  _read_first_datagram = false;
  _error = false;
  filename.set_binary();
  return filename.open_read(_in);
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::close
//       Access: Public
//  Description: Closes the file.  This is also implicitly done when
//               the DatagramInputFile destructs.
////////////////////////////////////////////////////////////////////
INLINE void DatagramInputFile::
close() {
  _in.close();
}
