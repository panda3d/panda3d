// Filename: compose_matrix_src.h
// Created by:  drose (21Feb99)
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

BEGIN_PUBLISH

INLINE_LINMATH void
compose_matrix(FLOATNAME(LMatrix3) &mat,
               const FLOATNAME(LVecBase3) &scale,
               const FLOATNAME(LVecBase3) &shear,
               const FLOATNAME(LVecBase3) &hpr,
               CoordinateSystem cs = CS_default);

INLINE_LINMATH void
compose_matrix(FLOATNAME(LMatrix4) &mat,
               const FLOATNAME(LVecBase3) &scale,
               const FLOATNAME(LVecBase3) &shear,
               const FLOATNAME(LVecBase3) &hpr,
               const FLOATNAME(LVecBase3) &translate,
               CoordinateSystem cs = CS_default);

INLINE_LINMATH void
compose_matrix(FLOATNAME(LMatrix4) &mat, 
               const FLOATTYPE components[num_matrix_components],
               CoordinateSystem cs = CS_default);

INLINE_LINMATH bool
decompose_matrix(const FLOATNAME(LMatrix3) &mat,
                 FLOATNAME(LVecBase3) &scale,
                 FLOATNAME(LVecBase3) &shear,
                 FLOATNAME(LVecBase3) &hpr,
                 CoordinateSystem cs = CS_default);

INLINE_LINMATH bool
decompose_matrix(const FLOATNAME(LMatrix4) &mat,
                 FLOATNAME(LVecBase3) &scale,
                 FLOATNAME(LVecBase3) &shear,
                 FLOATNAME(LVecBase3) &hpr,
                 FLOATNAME(LVecBase3) &translate,
                 CoordinateSystem cs = CS_default);

INLINE_LINMATH bool
decompose_matrix(const FLOATNAME(LMatrix4) &mat, FLOATTYPE components[num_matrix_components],
                 CoordinateSystem CS = CS_default);



// The following functions are deprecated; they have been replaced
// with new versions, above, that accept a shear component as well.

INLINE_LINMATH void
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

INLINE_LINMATH bool
decompose_matrix(const FLOATNAME(LMatrix3) &mat,
                 FLOATNAME(LVecBase3) &scale,
                 FLOATNAME(LVecBase3) &hpr,
                 CoordinateSystem cs = CS_default);

INLINE_LINMATH bool
decompose_matrix(const FLOATNAME(LMatrix4) &mat,
                 FLOATNAME(LVecBase3) &scale,
                 FLOATNAME(LVecBase3) &hpr,
                 FLOATNAME(LVecBase3) &translate,
                 CoordinateSystem cs = CS_default);


// The following functions are transitional and serve only to migrate
// code from the old, incorrect hpr calculations that Panda used to
// use.  New code should not call these functions directly; use the
// unqualified functions, above, instead.

EXPCL_PANDA void
compose_matrix_old_hpr(FLOATNAME(LMatrix3) &mat,
                       const FLOATNAME(LVecBase3) &scale,
                       const FLOATNAME(LVecBase3) &shear,
                       const FLOATNAME(LVecBase3) &hpr,
                       CoordinateSystem cs = CS_default);

INLINE_LINMATH void
compose_matrix_old_hpr(FLOATNAME(LMatrix4) &mat,
                       const FLOATNAME(LVecBase3) &scale,
                       const FLOATNAME(LVecBase3) &shear,
                       const FLOATNAME(LVecBase3) &hpr,
                       const FLOATNAME(LVecBase3) &translate,
                       CoordinateSystem cs = CS_default);

INLINE_LINMATH void
compose_matrix_old_hpr(FLOATNAME(LMatrix4) &mat, const FLOATTYPE components[num_matrix_components],
                       CoordinateSystem cs = CS_default);

EXPCL_PANDA bool
decompose_matrix_old_hpr(const FLOATNAME(LMatrix3) &mat,
                         FLOATNAME(LVecBase3) &scale,
                         FLOATNAME(LVecBase3) &shear,
                         FLOATNAME(LVecBase3) &hpr,
                         CoordinateSystem cs = CS_default);

INLINE_LINMATH bool
decompose_matrix_old_hpr(const FLOATNAME(LMatrix4) &mat,
                         FLOATNAME(LVecBase3) &scale,
                         FLOATNAME(LVecBase3) &shear,
                         FLOATNAME(LVecBase3) &hpr,
                         FLOATNAME(LVecBase3) &translate,
                         CoordinateSystem cs = CS_default);

INLINE_LINMATH bool
decompose_matrix_old_hpr(const FLOATNAME(LMatrix4) &mat, FLOATTYPE components[num_matrix_components],
                         CoordinateSystem CS = CS_default);


EXPCL_PANDA void
compose_matrix_new_hpr(FLOATNAME(LMatrix3) &mat,
                       const FLOATNAME(LVecBase3) &scale,
                       const FLOATNAME(LVecBase3) &shear,
                       const FLOATNAME(LVecBase3) &hpr,
                       CoordinateSystem cs = CS_default);

INLINE_LINMATH void
compose_matrix_new_hpr(FLOATNAME(LMatrix4) &mat,
                       const FLOATNAME(LVecBase3) &scale,
                       const FLOATNAME(LVecBase3) &shear,
                       const FLOATNAME(LVecBase3) &hpr,
                       const FLOATNAME(LVecBase3) &translate,
                       CoordinateSystem cs = CS_default);

INLINE_LINMATH void
compose_matrix_new_hpr(FLOATNAME(LMatrix4) &mat, const FLOATTYPE components[num_matrix_components],
                       CoordinateSystem cs = CS_default);

EXPCL_PANDA bool
decompose_matrix_new_hpr(const FLOATNAME(LMatrix3) &mat,
                         FLOATNAME(LVecBase3) &scale,
                         FLOATNAME(LVecBase3) &shear,
                         FLOATNAME(LVecBase3) &hpr,
                         CoordinateSystem cs = CS_default);

INLINE_LINMATH bool
decompose_matrix_new_hpr(const FLOATNAME(LMatrix4) &mat,
                         FLOATNAME(LVecBase3) &scale,
                         FLOATNAME(LVecBase3) &shear,
                         FLOATNAME(LVecBase3) &hpr,
                         FLOATNAME(LVecBase3) &translate,
                         CoordinateSystem cs = CS_default);

INLINE_LINMATH bool
decompose_matrix_new_hpr(const FLOATNAME(LMatrix4) &mat, FLOATTYPE components[num_matrix_components],
                         CoordinateSystem CS = CS_default);


EXPCL_PANDA FLOATNAME(LVecBase3)
old_to_new_hpr(const FLOATNAME(LVecBase3) &old_hpr);

END_PUBLISH

#include "compose_matrix_src.I"
