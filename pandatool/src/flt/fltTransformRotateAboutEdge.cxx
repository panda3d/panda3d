// Filename: fltTransformRotateAboutEdge.cxx
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

#include "fltTransformRotateAboutEdge.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"

TypeHandle FltTransformRotateAboutEdge::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRotateAboutEdge::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FltTransformRotateAboutEdge::
FltTransformRotateAboutEdge(FltHeader *header) : FltTransformRecord(header) {
  _point_a.set(0.0, 0.0, 0.0);
  _point_b.set(1.0, 0.0, 0.0);
  _angle = 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRotateAboutEdge::set
//       Access: Public
//  Description: Defines the rotation.  The angle is given in degrees,
//               counterclockwise about the axis as seen from point a.
////////////////////////////////////////////////////////////////////
void FltTransformRotateAboutEdge::
set(const LPoint3d &point_a, const LPoint3d &point_b, PN_stdfloat angle) {
  _point_a = point_a;
  _point_b = point_b;
  _angle = angle;

  recompute_matrix();
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRotateAboutEdge::get_point_a
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
const LPoint3d &FltTransformRotateAboutEdge::
get_point_a() const {
  return _point_a;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRotateAboutEdge::get_point_b
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
const LPoint3d &FltTransformRotateAboutEdge::
get_point_b() const {
  return _point_b;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRotateAboutEdge::get_angle
//       Access: Public
//  Description: Returns the angle of rotation, in degrees
//               counterclockwise about the axis as seen from point a.
////////////////////////////////////////////////////////////////////
PN_stdfloat FltTransformRotateAboutEdge::
get_angle() const {
  return _angle;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRotateAboutEdge::recompute_matrix
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void FltTransformRotateAboutEdge::
recompute_matrix() {
  if (_point_a == _point_b) {
    // Degenerate case.
    _matrix = LMatrix4d::ident_mat();
  } else {
    LVector3d axis = _point_b - _point_a;
    _matrix =
      LMatrix4d::translate_mat(-_point_a) *
      LMatrix4d::rotate_mat(_angle, normalize(axis), CS_zup_right) *
      LMatrix4d::translate_mat(_point_a);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRotateAboutEdge::extract_record
//       Access: Protected, Virtual
//  Description: Fills in the information in this record based on the
//               information given in the indicated datagram, whose
//               opcode has already been read.  Returns true on
//               success, false if the datagram is invalid.
////////////////////////////////////////////////////////////////////
bool FltTransformRotateAboutEdge::
extract_record(FltRecordReader &reader) {
  if (!FltTransformRecord::extract_record(reader)) {
    return false;
  }

  nassertr(reader.get_opcode() == FO_rotate_about_edge, false);
  DatagramIterator &iterator = reader.get_iterator();

  iterator.skip_bytes(4);   // Undocumented additional padding.

  _point_a[0] = iterator.get_be_float64();
  _point_a[1] = iterator.get_be_float64();
  _point_a[2] = iterator.get_be_float64();
  _point_b[0] = iterator.get_be_float64();
  _point_b[1] = iterator.get_be_float64();
  _point_b[2] = iterator.get_be_float64();
  _angle = iterator.get_be_float32();

  iterator.skip_bytes(4);   // Undocumented additional padding.

  recompute_matrix();

  check_remaining_size(iterator);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRotateAboutEdge::build_record
//       Access: Protected, Virtual
//  Description: Fills up the current record on the FltRecordWriter with
//               data for this record, but does not advance the
//               writer.  Returns true on success, false if there is
//               some error.
////////////////////////////////////////////////////////////////////
bool FltTransformRotateAboutEdge::
build_record(FltRecordWriter &writer) const {
  if (!FltTransformRecord::build_record(writer)) {
    return false;
  }

  writer.set_opcode(FO_rotate_about_edge);
  Datagram &datagram = writer.update_datagram();

  datagram.pad_bytes(4);   // Undocumented additional padding.

  datagram.add_be_float64(_point_a[0]);
  datagram.add_be_float64(_point_a[1]);
  datagram.add_be_float64(_point_a[2]);
  datagram.add_be_float64(_point_b[0]);
  datagram.add_be_float64(_point_b[1]);
  datagram.add_be_float64(_point_b[2]);
  datagram.add_be_float32(_angle);

  datagram.pad_bytes(4);   // Undocumented additional padding.

  return true;
}

