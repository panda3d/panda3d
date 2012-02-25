// Filename: datagramInputFile.I
// Created by:  drose (30Oct00)
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


////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DatagramInputFile::
DatagramInputFile() {
  _error = false;
  _read_first_datagram = false;
  _in = (istream *)NULL;
  _owns_in = false;
  _timestamp = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DatagramInputFile::
~DatagramInputFile() {
  close();
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::open
//       Access: Published
//  Description: Opens the indicated filename for reading.  Returns
//               true on success, false on failure.
////////////////////////////////////////////////////////////////////
INLINE bool DatagramInputFile::
open(const Filename &filename) {
  return open(new FileReference(filename));
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::get_stream
//       Access: Published
//  Description: Returns the istream represented by the input file.
////////////////////////////////////////////////////////////////////
INLINE istream &DatagramInputFile::
get_stream() {
  static ifstream null_stream;
  nassertr(_in != NULL, null_stream);
  return *_in;
}
