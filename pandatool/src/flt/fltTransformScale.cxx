/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltTransformScale.cxx
 * @author drose
 * @date 2000-08-30
 */

#include "fltTransformScale.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"

TypeHandle FltTransformScale::_type_handle;

/**

 */
FltTransformScale::
FltTransformScale(FltHeader *header) : FltTransformRecord(header) {
  _center.set(0.0, 0.0, 0.0);
  _scale.set(1.0, 1.0, 1.0);
}

/**
 * Defines the scale.
 */
void FltTransformScale::
set(const LPoint3d &center, const LVecBase3 &scale) {
  _center = center;
  _scale = scale;

  recompute_matrix();
}

/**
 * Returns true if the center is specified, false if it is not.  For some
 * reason, MultiGen stores large negative numbers in for the center if it is not
 * specified.  It is unclear what the purpose of this is.
 */
bool FltTransformScale::
has_center() const {
  return
    _center[0] > -1e+08 &&
    _center[1] > -1e+08 &&
    _center[2] > -1e+08;
}

/**

 */
const LPoint3d &FltTransformScale::
get_center() const {
  return _center;
}

/**

 */
const LVecBase3 &FltTransformScale::
get_scale() const {
  return _scale;
}

/**

 */
void FltTransformScale::
recompute_matrix() {
  if (has_center()) {
    _matrix =
      LMatrix4d::translate_mat(-_center) *
      LMatrix4d::scale_mat(LCAST(double, _scale)) *
      LMatrix4d::translate_mat(_center);
  } else {
    _matrix =
      LMatrix4d::scale_mat(LCAST(double, _scale));
  }
}

/**
 * Fills in the information in this record based on the information given in the
 * indicated datagram, whose opcode has already been read.  Returns true on
 * success, false if the datagram is invalid.
 */
bool FltTransformScale::
extract_record(FltRecordReader &reader) {
  if (!FltTransformRecord::extract_record(reader)) {
    return false;
  }

  nassertr(reader.get_opcode() == FO_scale, false);
  DatagramIterator &iterator = reader.get_iterator();

  iterator.skip_bytes(4);

  _center[0] = iterator.get_be_float64();
  _center[1] = iterator.get_be_float64();
  _center[2] = iterator.get_be_float64();
  _scale[0] = iterator.get_be_float32();
  _scale[1] = iterator.get_be_float32();
  _scale[2] = iterator.get_be_float32();

  iterator.skip_bytes(4);   // Undocumented additional padding.

  recompute_matrix();

  check_remaining_size(iterator);
  return true;
}

/**
 * Fills up the current record on the FltRecordWriter with data for this record,
 * but does not advance the writer.  Returns true on success, false if there is
 * some error.
 */
bool FltTransformScale::
build_record(FltRecordWriter &writer) const {
  if (!FltTransformRecord::build_record(writer)) {
    return false;
  }

  writer.set_opcode(FO_scale);
  Datagram &datagram = writer.update_datagram();

  datagram.pad_bytes(4);   // Undocumented additional padding.

  datagram.add_be_float64(_center[0]);
  datagram.add_be_float64(_center[1]);
  datagram.add_be_float64(_center[2]);
  datagram.add_be_float32(_scale[0]);
  datagram.add_be_float32(_scale[1]);
  datagram.add_be_float32(_scale[2]);

  datagram.pad_bytes(4);   // Undocumented additional padding.

  return true;
}
