// Filename: cast_to_float.h
// Created by:  drose (24May00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef CAST_TO_FLOAT_H
#define CAST_TO_FLOAT_H

#include "luse.h"

// The functions in this file are primarily for the benefit of a
// higher-level language that can't take advantage of the LCAST macro.
// These are a number of functions that convert our various math
// objects between floats and doubles.

INLINE_LINMATH LVecBase2f cast_to_float(const LVecBase2d &source);
INLINE_LINMATH LVecBase3f cast_to_float(const LVecBase3d &source);
INLINE_LINMATH LVecBase4f cast_to_float(const LVecBase4d &source);
INLINE_LINMATH LVector2f cast_to_float(const LVector2d &source);
INLINE_LINMATH LVector3f cast_to_float(const LVector3d &source);
INLINE_LINMATH LVector4f cast_to_float(const LVector4d &source);
INLINE_LINMATH LPoint2f cast_to_float(const LPoint2d &source);
INLINE_LINMATH LPoint3f cast_to_float(const LPoint3d &source);
INLINE_LINMATH LPoint4f cast_to_float(const LPoint4d &source);
INLINE_LINMATH LMatrix3f cast_to_float(const LMatrix3d &source);
INLINE_LINMATH LMatrix4f cast_to_float(const LMatrix4d &source);

#include "cast_to_float.I"

#endif


