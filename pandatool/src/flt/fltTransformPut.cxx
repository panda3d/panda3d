// Filename: fltTransformPut.cxx
// Created by:  drose (29Aug00)
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

#include "fltTransformPut.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"

#include "look_at.h"

TypeHandle FltTransformPut::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FltTransformPut::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FltTransformPut::
FltTransformPut(FltHeader *header) : FltTransformRecord(header) {
  _from_origin.set(0.0, 0.0, 0.0);
  _from_align.set(1.0, 0.0, 0.0);
  _from_track.set(1.0, 0.0, 0.0);
  _to_origin.set(0.0, 0.0, 0.0);
  _to_align.set(1.0, 0.0, 0.0);
  _to_track.set(1.0, 0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformPut::set
//       Access: Public
//  Description: Defines the put explicitly.  The transformation will
//               map the three "from" points to the corresponding
//               three "to" points.
////////////////////////////////////////////////////////////////////
void FltTransformPut::
set(const LPoint3d &from_origin, const LPoint3d &from_align,
    const LPoint3d &from_track,
    const LPoint3d &to_origin, const LPoint3d &to_align,
    const LPoint3d &to_track) {
  _from_origin = from_origin;
  _from_align = from_align;
  _from_track = from_track;
  _to_origin = to_origin;
  _to_align = to_align;
  _to_track = to_track;

  recompute_matrix();
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformPut::get_from_origin
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
const LPoint3d &FltTransformPut::
get_from_origin() const {
  return _from_origin;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformPut::get_from_align
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
const LPoint3d &FltTransformPut::
get_from_align() const {
  return _from_align;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformPut::get_from_track
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
const LPoint3d &FltTransformPut::
get_from_track() const {
  return _from_track;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformPut::get_to_origin
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
const LPoint3d &FltTransformPut::
get_to_origin() const {
  return _to_origin;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformPut::get_to_align
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
const LPoint3d &FltTransformPut::
get_to_align() const {
  return _to_align;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformPut::get_to_track
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
const LPoint3d &FltTransformPut::
get_to_track() const {
  return _to_track;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformPut::recompute_matrix
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void FltTransformPut::
recompute_matrix() {
  LMatrix4d r1, r2;
  look_at(r1, _from_align - _from_origin, _from_track - _from_origin, CS_zup_right);
  look_at(r2, _to_align - _to_origin, _to_track - _to_origin, CS_zup_right);

  _matrix =
    LMatrix4d::translate_mat(-_from_origin) *
    invert(r1) *
    r2 *
    LMatrix4d::translate_mat(_to_origin);
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformPut::extract_record
//       Access: Protected, Virtual
//  Description: Fills in the information in this record based on the
//               information given in the indicated datagram, whose
//               opcode has already been read.  Returns true on
//               success, false if the datagram is invalid.
////////////////////////////////////////////////////////////////////
bool FltTransformPut::
extract_record(FltRecordReader &reader) {
  if (!FltTransformRecord::extract_record(reader)) {
    return false;
  }

  nassertr(reader.get_opcode() == FO_put, false);
  DatagramIterator &iterator = reader.get_iterator();

  iterator.skip_bytes(4);   // Undocumented additional padding.

  _from_origin[0] = iterator.get_be_float64();
  _from_origin[1] = iterator.get_be_float64();
  _from_origin[2] = iterator.get_be_float64();
  _from_align[0] = iterator.get_be_float64();
  _from_align[1] = iterator.get_be_float64();
  _from_align[2] = iterator.get_be_float64();
  _from_track[0] = iterator.get_be_float64();
  _from_track[1] = iterator.get_be_float64();
  _from_track[2] = iterator.get_be_float64();
  _to_origin[0] = iterator.get_be_float64();
  _to_origin[1] = iterator.get_be_float64();
  _to_origin[2] = iterator.get_be_float64();
  _to_align[0] = iterator.get_be_float64();
  _to_align[1] = iterator.get_be_float64();
  _to_align[2] = iterator.get_be_float64();
  _to_track[0] = iterator.get_be_float64();
  _to_track[1] = iterator.get_be_float64();
  _to_track[2] = iterator.get_be_float64();

  recompute_matrix();

  check_remaining_size(iterator);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformPut::build_record
//       Access: Protected, Virtual
//  Description: Fills up the current record on the FltRecordWriter with
//               data for this record, but does not advance the
//               writer.  Returns true on success, false if there is
//               some error.
////////////////////////////////////////////////////////////////////
bool FltTransformPut::
build_record(FltRecordWriter &writer) const {
  if (!FltTransformRecord::build_record(writer)) {
    return false;
  }

  writer.set_opcode(FO_put);
  Datagram &datagram = writer.update_datagram();

  datagram.pad_bytes(4);   // Undocumented additional padding.

  datagram.add_be_float64(_from_origin[0]);
  datagram.add_be_float64(_from_origin[1]);
  datagram.add_be_float64(_from_origin[2]);
  datagram.add_be_float64(_from_align[0]);
  datagram.add_be_float64(_from_align[1]);
  datagram.add_be_float64(_from_align[2]);
  datagram.add_be_float64(_from_track[0]);
  datagram.add_be_float64(_from_track[1]);
  datagram.add_be_float64(_from_track[2]);
  datagram.add_be_float64(_to_origin[0]);
  datagram.add_be_float64(_to_origin[1]);
  datagram.add_be_float64(_to_origin[2]);
  datagram.add_be_float64(_to_align[0]);
  datagram.add_be_float64(_to_align[1]);
  datagram.add_be_float64(_to_align[2]);
  datagram.add_be_float64(_to_track[0]);
  datagram.add_be_float64(_to_track[1]);
  datagram.add_be_float64(_to_track[2]);

  return true;
}

