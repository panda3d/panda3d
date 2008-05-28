// Filename: fltTransformGeneralMatrix.cxx
// Created by:  drose (24Aug00)
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

#include "fltTransformGeneralMatrix.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"

TypeHandle FltTransformGeneralMatrix::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FltTransformGeneralMatrix::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FltTransformGeneralMatrix::
FltTransformGeneralMatrix(FltHeader *header) : FltTransformRecord(header) {
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformGeneralMatrix::set_matrix
//       Access: Public
//  Description: Directly sets the general matrix.
////////////////////////////////////////////////////////////////////
void FltTransformGeneralMatrix::
set_matrix(const LMatrix4d &matrix) {
  _matrix = matrix;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformGeneralMatrix::set_matrix
//       Access: Public
//  Description: Directly sets the general matrix.
////////////////////////////////////////////////////////////////////
void FltTransformGeneralMatrix::
set_matrix(const LMatrix4f &matrix) {
  _matrix = LCAST(double, matrix);
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformGeneralMatrix::extract_record
//       Access: Protected, Virtual
//  Description: Fills in the information in this record based on the
//               information given in the indicated datagram, whose
//               opcode has already been read.  Returns true on
//               success, false if the datagram is invalid.
////////////////////////////////////////////////////////////////////
bool FltTransformGeneralMatrix::
extract_record(FltRecordReader &reader) {
  if (!FltTransformRecord::extract_record(reader)) {
    return false;
  }

  nassertr(reader.get_opcode() == FO_general_matrix, false);
  DatagramIterator &iterator = reader.get_iterator();

  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      _matrix(r, c) = iterator.get_be_float32();
    }
  }

  check_remaining_size(iterator);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformGeneralMatrix::build_record
//       Access: Protected, Virtual
//  Description: Fills up the current record on the FltRecordWriter with
//               data for this record, but does not advance the
//               writer.  Returns true on success, false if there is
//               some error.
////////////////////////////////////////////////////////////////////
bool FltTransformGeneralMatrix::
build_record(FltRecordWriter &writer) const {
  if (!FltTransformRecord::build_record(writer)) {
    return false;
  }

  writer.set_opcode(FO_general_matrix);
  Datagram &datagram = writer.update_datagram();

  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      datagram.add_be_float32(_matrix(r, c));
    }
  }

  return true;
}
