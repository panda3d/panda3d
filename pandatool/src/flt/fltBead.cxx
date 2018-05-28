/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltBead.cxx
 * @author drose
 * @date 2000-08-24
 */

#include "fltBead.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"
#include "fltTransformGeneralMatrix.h"
#include "fltTransformPut.h"
#include "fltTransformRotateAboutEdge.h"
#include "fltTransformRotateAboutPoint.h"
#include "fltTransformScale.h"
#include "fltTransformTranslate.h"
#include "fltTransformRotateScale.h"
#include "config_flt.h"

#include "dcast.h"

#include <assert.h>

TypeHandle FltBead::_type_handle;

/**
 *
 */
FltBead::
FltBead(FltHeader *header) : FltRecord(header) {
  _has_transform = false;
  _transform = LMatrix4d::ident_mat();
  _replicate_count = 0;
}

/**
 * Returns true if the bead has been transformed, false otherwise.  If this
 * returns true, get_transform() will return the single-precision net
 * transformation, and get_num_transform_steps() will return nonzero.
 */
bool FltBead::
has_transform() const {
  return _has_transform;
}

/**
 * Returns the single-precision 4x4 matrix that represents the transform
 * applied to this bead, or the identity matrix if the bead has not been
 * transformed.
 */
const LMatrix4d &FltBead::
get_transform() const {
  return _has_transform ? _transform : LMatrix4d::ident_mat();
}

/**
 * Replaces the transform matrix on this bead.  This implicitly removes all of
 * the transform steps added previously, and replaces them with a single 4x4
 * general matrix transform step.
 */
void FltBead::
set_transform(const LMatrix4d &mat) {
  clear_transform();
  FltTransformGeneralMatrix *step = new FltTransformGeneralMatrix(_header);
  step->set_matrix(mat);
  add_transform_step(step);
}

/**
 * Removes any transform matrix and all transform steps on this bead.
 */
void FltBead::
clear_transform() {
  _has_transform = false;
  _transform = LMatrix4d::ident_mat();
  _transform_steps.clear();
}

/**
 * Returns the number of individual steps that define the net transform on
 * this bead as returned by set_transform().  Each step is a single
 * transformation; the concatenation of all transformations will produce the
 * matrix represented by set_transform().
 */
int FltBead::
get_num_transform_steps() const {
  return _transform_steps.size();
}

/**
 * Returns the nth individual step that defines the net transform on this
 * bead.  See get_num_transform_steps().
 */
FltTransformRecord *FltBead::
get_transform_step(int n) {
  nassertr(n >= 0 && n < (int)_transform_steps.size(),
           nullptr);
  return _transform_steps[n];
}

/**
 * Returns the nth individual step that defines the net transform on this
 * bead.  See get_num_transform_steps().
 */
const FltTransformRecord *FltBead::
get_transform_step(int n) const {
  nassertr(n >= 0 && n < (int)_transform_steps.size(),
           nullptr);
  return _transform_steps[n];
}

/**
 * Applies the indicated transform step to the net transformation applied to
 * the bead.
 */
void FltBead::
add_transform_step(FltTransformRecord *record) {
  if (!_has_transform) {
    _has_transform = true;
    _transform = record->get_matrix();
  } else {
    _transform = record->get_matrix() * _transform;
  }
  _transform_steps.push_back(record);
}

/**
 * Returns the replicate count of this bead.  If this is nonzero, it means
 * that the bead is implicitly copied this number of additional times (for
 * replicate_count + 1 total copies), applying the transform on this bead for
 * each copy.  In this case, the transform does *not* apply to the initial
 * copy of the bead.
 */
int FltBead::
get_replicate_count() const {
  return _replicate_count;
}

/**
 * Changes the replicate count of this bead.  If you are setting the replicate
 * count to some nonzero number, you must also set a transform on the bead.
 * See set_replicate_count().
 */
void FltBead::
set_replicate_count(int count) {
  _replicate_count = count;
}

/**
 * Fills in the information in this bead based on the information given in the
 * indicated datagram, whose opcode has already been read.  Returns true on
 * success, false if the datagram is invalid.
 */
bool FltBead::
extract_record(FltRecordReader &reader) {
  if (!FltRecord::extract_record(reader)) {
    return false;
  }
  return true;
}

/**
 * Checks whether the given bead, which follows this bead sequentially in the
 * file, is an ancillary record of this bead.  If it is, extracts the relevant
 * information and returns true; otherwise, leaves it alone and returns false.
 */
bool FltBead::
extract_ancillary(FltRecordReader &reader) {
  FltTransformRecord *step = nullptr;

  switch (reader.get_opcode()) {
  case FO_transform_matrix:
    return extract_transform_matrix(reader);

  case FO_general_matrix:
    step = new FltTransformGeneralMatrix(_header);
    break;

  case FO_put:
    step = new FltTransformPut(_header);
    break;

  case FO_rotate_about_edge:
    step = new FltTransformRotateAboutEdge(_header);
    break;

  case FO_rotate_about_point:
    step = new FltTransformRotateAboutPoint(_header);
    break;

  case FO_scale:
    step = new FltTransformScale(_header);
    break;

  case FO_translate:
    step = new FltTransformTranslate(_header);
    break;

  case FO_rotate_and_scale:
    step = new FltTransformRotateScale(_header);
    break;

  case FO_replicate:
    return extract_replicate_count(reader);

  default:
    return FltRecord::extract_ancillary(reader);
  }

  // A transform step.
  nassertr(step != nullptr, false);
  if (!step->extract_record(reader)) {
    return false;
  }
  _transform_steps.push_back(DCAST(FltTransformRecord, step));

  return true;
}

/**
 * Fills up the current record on the FltRecordWriter with data for this
 * record, but does not advance the writer.  Returns true on success, false if
 * there is some error.
 */
bool FltBead::
build_record(FltRecordWriter &writer) const {
  if (!FltRecord::build_record(writer)) {
    return false;
  }
  return true;
}

/**
 * Writes whatever ancillary records are required for this record.  Returns
 * FE_ok on success, or something else if there is some error.
 */
FltError FltBead::
write_ancillary(FltRecordWriter &writer) const {
  if (_has_transform) {
    FltError result = write_transform(writer);
    if (result != FE_ok) {
      return result;
    }
  }
  if (_replicate_count != 0) {
    FltError result = write_replicate_count(writer);
    if (result != FE_ok) {
      return result;
    }
  }


  return FltRecord::write_ancillary(writer);
}

/**
 * Reads a transform matrix ancillary bead.  This defines the net
 * transformation that has been applied to the bead, and precedes the set of
 * individual transform steps that define how this net transform was computed.
 */
bool FltBead::
extract_transform_matrix(FltRecordReader &reader) {
  nassertr(reader.get_opcode() == FO_transform_matrix, false);
  DatagramIterator &iterator = reader.get_iterator();

  LMatrix4d matrix;
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      matrix(r, c) = iterator.get_be_float32();
    }
  }
  check_remaining_size(iterator);

  _transform_steps.clear();
  _has_transform = true;
  _transform = matrix;

  return true;
}

/**
 * Reads a replicate count ancillary bead.
 */
bool FltBead::
extract_replicate_count(FltRecordReader &reader) {
  nassertr(reader.get_opcode() == FO_replicate, false);
  DatagramIterator &iterator = reader.get_iterator();

  _replicate_count = iterator.get_be_int16();
  iterator.skip_bytes(2);

  check_remaining_size(iterator);
  return true;
}

/**
 * Writes out the transformation and all of its defining steps.
 */
FltError FltBead::
write_transform(FltRecordWriter &writer) const {
  // First, write out the initial transform indication.
  writer.set_opcode(FO_transform_matrix);
  Datagram &datagram = writer.update_datagram();

  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      datagram.add_be_float32(_transform(r, c));
    }
  }

  FltError result = writer.advance();
  if (result != FE_ok) {
    return result;
  }

  // Now, write out each of the steps of the transform.
  Transforms::const_iterator ti;
  for (ti = _transform_steps.begin(); ti != _transform_steps.end(); ++ti) {
    if (!(*ti)->build_record(writer)) {
      assert(!flt_error_abort);
      return FE_invalid_record;
    }
    FltError result = writer.advance();
    if (result != FE_ok) {
      return result;
    }
  }

  return FE_ok;
}

/**
 * Writes out the replicate count, if needed.
 */
FltError FltBead::
write_replicate_count(FltRecordWriter &writer) const {
  if (_replicate_count != 0) {
    writer.set_opcode(FO_replicate);
    Datagram &datagram = writer.update_datagram();

    datagram.add_be_int16(_replicate_count);
    datagram.pad_bytes(2);

    FltError result = writer.advance();
    if (result != FE_ok) {
      return result;
    }
  }

  return FE_ok;
}
