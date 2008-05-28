// Filename: fltTransformTranslate.cxx
// Created by:  drose (30Aug00)
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

#include "fltTransformTranslate.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"

TypeHandle FltTransformTranslate::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FltTransformTranslate::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FltTransformTranslate::
FltTransformTranslate(FltHeader *header) : FltTransformRecord(header) {
  _from.set(0.0, 0.0, 0.0);
  _delta.set(0.0, 0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformTranslate::set
//       Access: Public
//  Description: Defines the translation.  The "from" point seems to
//               be pretty much ignored.
////////////////////////////////////////////////////////////////////
void FltTransformTranslate::
set(const LPoint3d &from, const LVector3d &delta) {
  _from = from;
  _delta = delta;

  recompute_matrix();
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformTranslate::get_from
//       Access: Public
//  Description: Returns the reference point of the translation.  This
//               is largely meaningless.
////////////////////////////////////////////////////////////////////
const LPoint3d &FltTransformTranslate::
get_from() const {
  return _from;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformTranslate::get_delta
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
const LVector3d &FltTransformTranslate::
get_delta() const {
  return _delta;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformTranslate::recompute_matrix
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void FltTransformTranslate::
recompute_matrix() {
  _matrix = LMatrix4d::translate_mat(_delta);
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformTranslate::extract_record
//       Access: Protected, Virtual
//  Description: Fills in the information in this record based on the
//               information given in the indicated datagram, whose
//               opcode has already been read.  Returns true on
//               success, false if the datagram is invalid.
////////////////////////////////////////////////////////////////////
bool FltTransformTranslate::
extract_record(FltRecordReader &reader) {
  if (!FltTransformRecord::extract_record(reader)) {
    return false;
  }

  nassertr(reader.get_opcode() == FO_translate, false);
  DatagramIterator &iterator = reader.get_iterator();

  iterator.skip_bytes(4);   // Undocumented additional padding.

  _from[0] = iterator.get_be_float64();
  _from[1] = iterator.get_be_float64();
  _from[2] = iterator.get_be_float64();
  _delta[0] = iterator.get_be_float64();
  _delta[1] = iterator.get_be_float64();
  _delta[2] = iterator.get_be_float64();

  //  iterator.skip_bytes(4);   // Undocumented additional padding.

  recompute_matrix();

  check_remaining_size(iterator);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformTranslate::build_record
//       Access: Protected, Virtual
//  Description: Fills up the current record on the FltRecordWriter with
//               data for this record, but does not advance the
//               writer.  Returns true on success, false if there is
//               some error.
////////////////////////////////////////////////////////////////////
bool FltTransformTranslate::
build_record(FltRecordWriter &writer) const {
  if (!FltTransformRecord::build_record(writer)) {
    return false;
  }

  writer.set_opcode(FO_translate);
  Datagram &datagram = writer.update_datagram();

  datagram.pad_bytes(4);   // Undocumented additional padding.

  datagram.add_be_float64(_from[0]);
  datagram.add_be_float64(_from[1]);
  datagram.add_be_float64(_from[2]);
  datagram.add_be_float64(_delta[0]);
  datagram.add_be_float64(_delta[1]);
  datagram.add_be_float64(_delta[2]);

  //  datagram.pad_bytes(4);   // Undocumented additional padding.

  return true;
}

