// Filename: fltVectorRecord.cxx
// Created by:  drose (30Aug02)
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

#include "fltVectorRecord.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"

TypeHandle FltVectorRecord::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FltVectorRecord::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FltVectorRecord::
FltVectorRecord(FltHeader *header) : FltRecord(header) {
  _vector.set(0.0f, 0.0f, 0.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: FltVectorRecord::get_vector
//       Access: Public
//  Description: Returns the vector value.
////////////////////////////////////////////////////////////////////
const LVector3 &FltVectorRecord::
get_vector() const {
  return _vector;
}

////////////////////////////////////////////////////////////////////
//     Function: FltVectorRecord::extract_record
//       Access: Protected, Virtual
//  Description: Fills in the information in this record based on the
//               information given in the indicated datagram, whose
//               opcode has already been read.  Returns true on
//               success, false if the datagram is invalid.
////////////////////////////////////////////////////////////////////
bool FltVectorRecord::
extract_record(FltRecordReader &reader) {
  if (!FltRecord::extract_record(reader)) {
    return false;
  }

  nassertr(reader.get_opcode() == FO_vector, false);
  DatagramIterator &iterator = reader.get_iterator();

  _vector[0] = iterator.get_be_float32();
  _vector[1] = iterator.get_be_float32();
  _vector[2] = iterator.get_be_float32();

  check_remaining_size(iterator);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FltVectorRecord::build_record
//       Access: Protected, Virtual
//  Description: Fills up the current record on the FltRecordWriter with
//               data for this record, but does not advance the
//               writer.  Returns true on success, false if there is
//               some error.
////////////////////////////////////////////////////////////////////
bool FltVectorRecord::
build_record(FltRecordWriter &writer) const {
  if (!FltRecord::build_record(writer)) {
    return false;
  }

  writer.set_opcode(FO_vector);
  Datagram &datagram = writer.update_datagram();

  datagram.add_be_float32(_vector[0]);
  datagram.add_be_float32(_vector[1]);
  datagram.add_be_float32(_vector[2]);

  return true;
}

