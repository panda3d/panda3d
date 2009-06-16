// Filename: p3dMultifileReader.I
// Created by:  drose (15Jun09)
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
//     Function: P3DMultifileReader::read_uint16
//       Access: Private
//  Description: Extracts an unsigned short from the file.
////////////////////////////////////////////////////////////////////
inline unsigned int P3DMultifileReader::
read_uint16() {
  unsigned int a = _in.get();
  unsigned int b = _in.get();
  return (b << 8) | a;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMultifileReader::read_uint32
//       Access: Private
//  Description: Extracts an unsigned long from the file.
////////////////////////////////////////////////////////////////////
inline unsigned int P3DMultifileReader::
read_uint32() {
  unsigned int a = _in.get();
  unsigned int b = _in.get();
  unsigned int c = _in.get();
  unsigned int d = _in.get();
  return (d << 24) | (c << 16) | (b << 8) | a;
}
