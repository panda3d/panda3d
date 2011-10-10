// Filename: fltTransformRotateAboutPoint.cxx
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

#include "fltTransformRotateAboutPoint.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"

TypeHandle FltTransformRotateAboutPoint::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRotateAboutPoint::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FltTransformRotateAboutPoint::
FltTransformRotateAboutPoint(FltHeader *header) : FltTransformRecord(header) {
  _center.set(0.0, 0.0, 0.0);
  _axis.set(1.0, 0.0, 0.0);
  _angle = 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRotateAboutPoint::set
//       Access: Public
//  Description: Defines the rotation.  The angle is given in degrees,
//               counterclockwise about the axis as seen from point a.
////////////////////////////////////////////////////////////////////
void FltTransformRotateAboutPoint::
set(const LPoint3d &center, const LVector3 &axis, PN_stdfloat angle) {
  _center = center;
  _axis = axis;
  _angle = angle;

  recompute_matrix();
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRotateAboutPoint::get_center
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
const LPoint3d &FltTransformRotateAboutPoint::
get_center() const {
  return _center;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRotateAboutPoint::get_axis
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
const LVector3 &FltTransformRotateAboutPoint::
get_axis() const {
  return _axis;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRotateAboutPoint::get_angle
//       Access: Public
//  Description: Returns the angle of rotation, in degrees
//               counterclockwise about the axis.
////////////////////////////////////////////////////////////////////
PN_stdfloat FltTransformRotateAboutPoint::
get_angle() const {
  return _angle;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRotateAboutPoint::recompute_matrix
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void FltTransformRotateAboutPoint::
recompute_matrix() {
  if (_axis == LVector3::zero()) {
    // Degenerate case.
    _matrix = LMatrix4d::ident_mat();
  } else {
    LVector3d axis = LCAST(double, _axis);

    _matrix =
      LMatrix4d::translate_mat(-_center) *
      LMatrix4d::rotate_mat(_angle, axis, CS_zup_right) *
      LMatrix4d::translate_mat(_center);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRotateAboutPoint::extract_record
//       Access: Protected, Virtual
//  Description: Fills in the information in this record based on the
//               information given in the indicated datagram, whose
//               opcode has already been read.  Returns true on
//               success, false if the datagram is invalid.
////////////////////////////////////////////////////////////////////
bool FltTransformRotateAboutPoint::
extract_record(FltRecordReader &reader) {
  if (!FltTransformRecord::extract_record(reader)) {
    return false;
  }

  nassertr(reader.get_opcode() == FO_rotate_about_point, false);
  DatagramIterator &iterator = reader.get_iterator();

  iterator.skip_bytes(4);   // Undocumented additional padding.

  _center[0] = iterator.get_be_float64();
  _center[1] = iterator.get_be_float64();
  _center[2] = iterator.get_be_float64();
  _axis[0] = iterator.get_be_float32();
  _axis[1] = iterator.get_be_float32();
  _axis[2] = iterator.get_be_float32();
  _angle = iterator.get_be_float32();

  recompute_matrix();

  check_remaining_size(iterator);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRotateAboutPoint::build_record
//       Access: Protected, Virtual
//  Description: Fills up the current record on the FltRecordWriter with
//               data for this record, but does not advance the
//               writer.  Returns true on success, false if there is
//               some error.
////////////////////////////////////////////////////////////////////
bool FltTransformRotateAboutPoint::
build_record(FltRecordWriter &writer) const {
  if (!FltTransformRecord::build_record(writer)) {
    return false;
  }

  writer.set_opcode(FO_rotate_about_point);
  Datagram &datagram = writer.update_datagram();

  datagram.pad_bytes(4);   // Undocumented additional padding.

  datagram.add_be_float64(_center[0]);
  datagram.add_be_float64(_center[1]);
  datagram.add_be_float64(_center[2]);
  datagram.add_be_float32(_axis[0]);
  datagram.add_be_float32(_axis[1]);
  datagram.add_be_float32(_axis[2]);
  datagram.add_be_float32(_angle);

  return true;
}

