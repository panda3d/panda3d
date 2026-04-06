/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cast_to_double.h
 * @author drose
 * @date 2000-05-24
 */

#ifndef CAST_TO_DOUBLE_H
#define CAST_TO_DOUBLE_H

#include "luse.h"

// The functions in this file are primarily for the benefit of a higher-level
// language that can't take advantage of the LCAST macro.  These are a number
// of functions that convert our various math objects between floats and
// doubles.

INLINE_LINMATH LVecBase2d cast_to_double(const LVecBase2f &source);
INLINE_LINMATH LVecBase3d cast_to_double(const LVecBase3f &source);
INLINE_LINMATH LVecBase4d cast_to_double(const LVecBase4f &source);
INLINE_LINMATH LVector2d cast_to_double(const LVector2f &source);
INLINE_LINMATH LVector3d cast_to_double(const LVector3f &source);
INLINE_LINMATH LVector4d cast_to_double(const LVector4f &source);
INLINE_LINMATH LPoint2d cast_to_double(const LPoint2f &source);
INLINE_LINMATH LPoint3d cast_to_double(const LPoint3f &source);
INLINE_LINMATH LPoint4d cast_to_double(const LPoint4f &source);
INLINE_LINMATH LMatrix3d cast_to_double(const LMatrix3f &source);
INLINE_LINMATH LMatrix4d cast_to_double(const LMatrix4f &source);

#include "cast_to_double.I"

#endif
