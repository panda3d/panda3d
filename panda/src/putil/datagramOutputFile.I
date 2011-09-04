// Filename: datagramOutputFile.I
// Created by:  drose (27Oct00)
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
//     Function: DatagramOutputFile::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DatagramOutputFile::
DatagramOutputFile() {
  _error = false;
  _wrote_first_datagram = false;
  _out = (ostream *)NULL;
  _owns_out = false;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramOutputFile::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DatagramOutputFile::
~DatagramOutputFile() {
  close();
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramOutputFile::open
//       Access: Published
//  Description: Opens the indicated filename for writing.  Returns
//               true on success, false on failure.
////////////////////////////////////////////////////////////////////
INLINE bool DatagramOutputFile::
open(const Filename &filename) {
  return open(new FileReference(filename));
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramOutputFile::get_stream
//       Access: Published
//  Description: Returns the ostream represented by the output file.
////////////////////////////////////////////////////////////////////
INLINE ostream &DatagramOutputFile::
get_stream() {
  static ofstream null_stream;
  nassertr(_out != NULL, null_stream);
  return *_out;
}
