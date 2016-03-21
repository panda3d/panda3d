/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltTransformRecord.cxx
 * @author drose
 * @date 2000-08-24
 */

#include "fltTransformRecord.h"

TypeHandle FltTransformRecord::_type_handle;

/**
 *
 */
FltTransformRecord::
FltTransformRecord(FltHeader *header) : FltRecord(header) {
  _matrix = LMatrix4d::ident_mat();
}

/**
 * Returns the transform matrix represented by this particular component of
 * the transform.
 */
const LMatrix4d &FltTransformRecord::
get_matrix() const {
  return _matrix;
}
