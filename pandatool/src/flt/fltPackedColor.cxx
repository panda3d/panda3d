// Filename: fltPackedColor.cxx
// Created by:  drose (25Aug00)
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

#include "fltPackedColor.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"

////////////////////////////////////////////////////////////////////
//     Function: FltPackedColor::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void FltPackedColor::
output(ostream &out) const {
  out << "(" << _r << " " << _g << " " << _b << " " << _a << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: FltPackedColor::extract_record
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool FltPackedColor::
extract_record(FltRecordReader &reader) {
  DatagramIterator &iterator = reader.get_iterator();

  _a = iterator.get_uint8();
  _b = iterator.get_uint8();
  _g = iterator.get_uint8();
  _r = iterator.get_uint8();

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FltPackedColor::build_record
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool FltPackedColor::
build_record(FltRecordWriter &writer) const {
  Datagram &datagram = writer.update_datagram();

  datagram.add_uint8(_a);
  datagram.add_uint8(_b);
  datagram.add_uint8(_g);
  datagram.add_uint8(_r);

  return true;
}
