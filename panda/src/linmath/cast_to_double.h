// Filename: cast_to_double.h
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

#ifndef CAST_TO_DOUBLE_H
#define CAST_TO_DOUBLE_H

#include "luse.h"

// The functions in this file are primarily for the benefit of a
// higher-level language that can't take advantage of the LCAST macro.
// These are a number of functions that convert our various math
// objects between floats and doubles.

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


