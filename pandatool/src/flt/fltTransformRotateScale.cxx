// Filename: fltTransformRotateScale.cxx
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

#include "fltTransformRotateScale.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"

#include "mathNumbers.h"
#include "look_at.h"

TypeHandle FltTransformRotateScale::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRotateScale::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FltTransformRotateScale::
FltTransformRotateScale(FltHeader *header) : FltTransformRecord(header) {
  _center.set(0.0, 0.0, 0.0);
  _reference_point.set(0.0, 0.0, 0.0);
  _to_point.set(0.0, 0.0, 0.0);
  _overall_scale = 1.0;
  _axis_scale = 1.0;
  _angle = 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRotateScale::set
//       Access: Public
//  Description: Defines the transform explicitly.  The angle of
//               rotation is determined by the angle between the
//               reference point and the to point (relative to the
//               center), and the scale factor is determined by the
//               distance between the reference point and the center
//               point.  If axis_scale is true, the scale is along
//               reference point axis only; otherwise, it is a uniform
//               scale.
////////////////////////////////////////////////////////////////////
void FltTransformRotateScale::
set(const LPoint3d &center, const LPoint3d &reference_point,
    const LPoint3d &to_point, bool axis_scale) {
  _center = center;
  _reference_point = reference_point;
  _to_point = to_point;

  LVector3d v1 = _reference_point - _center;
  LVector3d v2 = _to_point - _center;

  _angle =
    acos(dot(normalize(v1), normalize(v2))) * 180.0 / MathNumbers::pi;

  if (axis_scale) {
    _axis_scale = length(v1);
    _overall_scale = 1.0;
  } else {
    _overall_scale = length(v1);
    _axis_scale = 1.0;
  }

  recompute_matrix();
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRotateScale::get_center
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
const LPoint3d &FltTransformRotateScale::
get_center() const {
  return _center;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRotateScale::get_reference_point
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
const LPoint3d &FltTransformRotateScale::
get_reference_point() const {
  return _reference_point;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRotateScale::get_to_point
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
const LPoint3d &FltTransformRotateScale::
get_to_point() const {
  return _to_point;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRotateScale::get_overall_scale
//       Access: Public
//  Description: Returns the overall scale factor.
////////////////////////////////////////////////////////////////////
PN_stdfloat FltTransformRotateScale::
get_overall_scale() const {
  return _overall_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRotateScale::get_axis_scale
//       Access: Public
//  Description: Returns the scale factor in the direction of the
//               axis.
////////////////////////////////////////////////////////////////////
PN_stdfloat FltTransformRotateScale::
get_axis_scale() const {
  return _axis_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRotateScale::get_angle
//       Access: Public
//  Description: Returns the angle of rotation in degrees.
////////////////////////////////////////////////////////////////////
PN_stdfloat FltTransformRotateScale::
get_angle() const {
  return _angle;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRotateScale::recompute_matrix
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void FltTransformRotateScale::
recompute_matrix() {
  LVector3d v1 = _reference_point - _center;
  LVector3d v2 = _to_point - _center;
  LVector3d rotate_axis = normalize(cross(v1, v2));

  // To scale along an axis, we have to do a bit of work.  First
  // determine the matrices to rotate and unrotate the rotate axis
  // to the y-forward axis.
  LMatrix4d r1;
  look_at(r1, v1, rotate_axis, CS_zup_right);

  _matrix =
    LMatrix4d::translate_mat(-_center) *
    r1 *
    LMatrix4d::scale_mat(1.0, _axis_scale, 1.0) *
    LMatrix4d::scale_mat(_overall_scale) *
    invert(r1) *
    LMatrix4d::rotate_mat(_angle, rotate_axis, CS_zup_right) *
    LMatrix4d::translate_mat(_center);
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRotateScale::extract_record
//       Access: Protected, Virtual
//  Description: Fills in the information in this record based on the
//               information given in the indicated datagram, whose
//               opcode has already been read.  Returns true on
//               success, false if the datagram is invalid.
////////////////////////////////////////////////////////////////////
bool FltTransformRotateScale::
extract_record(FltRecordReader &reader) {
  if (!FltTransformRecord::extract_record(reader)) {
    return false;
  }

  nassertr(reader.get_opcode() == FO_rotate_and_scale, false);
  DatagramIterator &iterator = reader.get_iterator();

  iterator.skip_bytes(4);   // Undocumented additional padding.

  _center[0] = iterator.get_be_float64();
  _center[1] = iterator.get_be_float64();
  _center[2] = iterator.get_be_float64();
  _reference_point[0] = iterator.get_be_float64();
  _reference_point[1] = iterator.get_be_float64();
  _reference_point[2] = iterator.get_be_float64();
  _to_point[0] = iterator.get_be_float64();
  _to_point[1] = iterator.get_be_float64();
  _to_point[2] = iterator.get_be_float64();
  _overall_scale = iterator.get_be_float32();
  _axis_scale = iterator.get_be_float32();
  _angle = iterator.get_be_float32();

  iterator.skip_bytes(4);   // Undocumented additional padding.

  recompute_matrix();

  check_remaining_size(iterator);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FltTransformRotateScale::build_record
//       Access: Protected, Virtual
//  Description: Fills up the current record on the FltRecordWriter with
//               data for this record, but does not advance the
//               writer.  Returns true on success, false if there is
//               some error.
////////////////////////////////////////////////////////////////////
bool FltTransformRotateScale::
build_record(FltRecordWriter &writer) const {
  if (!FltTransformRecord::build_record(writer)) {
    return false;
  }

  writer.set_opcode(FO_put);
  Datagram &datagram = writer.update_datagram();

  datagram.pad_bytes(4);   // Undocumented additional padding.

  datagram.add_be_float64(_center[0]);
  datagram.add_be_float64(_center[1]);
  datagram.add_be_float64(_center[2]);
  datagram.add_be_float64(_reference_point[0]);
  datagram.add_be_float64(_reference_point[1]);
  datagram.add_be_float64(_reference_point[2]);
  datagram.add_be_float64(_to_point[0]);
  datagram.add_be_float64(_to_point[1]);
  datagram.add_be_float64(_to_point[2]);
  datagram.add_be_float32(_overall_scale);
  datagram.add_be_float32(_axis_scale);
  datagram.add_be_float32(_angle);

  datagram.pad_bytes(4);   // Undocumented additional padding.

  return true;
}

