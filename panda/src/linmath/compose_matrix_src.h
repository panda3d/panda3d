// Filename: compose_matrix_src.h
// Created by:  drose (21Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

BEGIN_PUBLISH

EXPCL_PANDA void
compose_matrix(FLOATNAME(LMatrix3) &mat,
               const FLOATNAME(LVecBase3) &scale,
               const FLOATNAME(LVecBase3) &hpr,
               CoordinateSystem cs = CS_default);

INLINE_LINMATH void
compose_matrix(FLOATNAME(LMatrix4) &mat,
               const FLOATNAME(LVecBase3) &scale,
               const FLOATNAME(LVecBase3) &hpr,
               const FLOATNAME(LVecBase3) &translate,
               CoordinateSystem cs = CS_default);

INLINE_LINMATH void
compose_matrix(FLOATNAME(LMatrix4) &mat, const FLOATTYPE components[9],
               CoordinateSystem cs = CS_default);

EXPCL_PANDA bool
decompose_matrix(const FLOATNAME(LMatrix3) &mat,
                 FLOATNAME(LVecBase3) &scale,
                 FLOATNAME(LVecBase3) &hpr,
                 CoordinateSystem cs = CS_default);

EXPCL_PANDA bool
decompose_matrix(const FLOATNAME(LMatrix3) &mat,
                 FLOATNAME(LVecBase3) &scale,
                 FLOATNAME(LVecBase3) &hpr,
                 FLOATTYPE roll,
                 CoordinateSystem cs);

INLINE_LINMATH bool
decompose_matrix(const FLOATNAME(LMatrix4) &mat,
                 FLOATNAME(LVecBase3) &scale,
                 FLOATNAME(LVecBase3) &hpr,
                 FLOATNAME(LVecBase3) &translate,
                 CoordinateSystem cs = CS_default);

INLINE_LINMATH bool
decompose_matrix(const FLOATNAME(LMatrix4) &mat,
                 FLOATNAME(LVecBase3) &scale,
                 FLOATNAME(LVecBase3) &hpr,
                 FLOATNAME(LVecBase3) &translate,
                 FLOATTYPE roll,
                 CoordinateSystem cs);

INLINE_LINMATH bool
decompose_matrix(const FLOATNAME(LMatrix4) &mat, FLOATTYPE components[9],
                 CoordinateSystem CS = CS_default);

END_PUBLISH

#include "compose_matrix_src.I"
