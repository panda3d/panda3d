/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cast_to_float.I
 * @author drose
 * @date 2000-05-24
 */

INLINE_LINMATH LVecBase2f cast_to_float(const LVecBase2d &source) {
  return LCAST(float, source);
}

INLINE_LINMATH LVecBase3f cast_to_float(const LVecBase3d &source) {
  return LCAST(float, source);
}

INLINE_LINMATH LVecBase4f cast_to_float(const LVecBase4d &source) {
  return LCAST(float, source);
}

INLINE_LINMATH LVector2f cast_to_float(const LVector2d &source) {
  return LCAST(float, source);
}

INLINE_LINMATH LVector3f cast_to_float(const LVector3d &source) {
  return LCAST(float, source);
}

INLINE_LINMATH LVector4f cast_to_float(const LVector4d &source) {
  return LCAST(float, source);
}

INLINE_LINMATH LPoint2f cast_to_float(const LPoint2d &source) {
  return LCAST(float, source);
}

INLINE_LINMATH LPoint3f cast_to_float(const LPoint3d &source) {
  return LCAST(float, source);
}

INLINE_LINMATH LPoint4f cast_to_float(const LPoint4d &source) {
  return LCAST(float, source);
}

INLINE_LINMATH LMatrix3f cast_to_float(const LMatrix3d &source) {
  return LCAST(float, source);
}

INLINE_LINMATH LMatrix4f cast_to_float(const LMatrix4d &source) {
  return LCAST(float, source);
}
