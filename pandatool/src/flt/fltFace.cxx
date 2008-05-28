// Filename: fltFace.cxx
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

#include "fltFace.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"
#include "fltHeader.h"
#include "fltMaterial.h"

TypeHandle FltFace::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FltFace::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FltFace::
FltFace(FltHeader *header) : FltGeometry(header) {
}

////////////////////////////////////////////////////////////////////
//     Function: FltFace::extract_record
//       Access: Protected, Virtual
//  Description: Fills in the information in this bead based on the
//               information given in the indicated datagram, whose
//               opcode has already been read.  Returns true on
//               success, false if the datagram is invalid.
////////////////////////////////////////////////////////////////////
bool FltFace::
extract_record(FltRecordReader &reader) {
  if (!FltBeadID::extract_record(reader)) {
    return false;
  }
  if (!FltGeometry::extract_record(reader)) {
    return false;
  }

  nassertr(reader.get_opcode() == FO_face, false);

  DatagramIterator &iterator = reader.get_iterator();
  check_remaining_size(iterator);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FltFace::build_record
//       Access: Protected, Virtual
//  Description: Fills up the current record on the FltRecordWriter with
//               data for this record, but does not advance the
//               writer.  Returns true on success, false if there is
//               some error.
////////////////////////////////////////////////////////////////////
bool FltFace::
build_record(FltRecordWriter &writer) const {
  if (!FltBeadID::build_record(writer)) {
    return false;
  }
  if (!FltGeometry::build_record(writer)) {
    return false;
  }

  writer.set_opcode(FO_face);

  return true;
}
