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
