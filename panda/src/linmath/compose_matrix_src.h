/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file compose_matrix_src.h
 * @author drose
 * @date 1999-02-21
 */

BEGIN_PUBLISH

EXPCL_PANDA_LINMATH void
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

EXPCL_PANDA_LINMATH bool
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
decompose_matrix(const FLOATNAME(LMatrix4) &mat,
                 FLOATTYPE components[num_matrix_components],
                 CoordinateSystem CS = CS_default);



// The following functions are deprecated; they have been replaced with new
// versions, above, that accept a shear component as well.

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


// The following functions are transitional and serve only to migrate code
// from the old, incorrect hpr calculations that Panda used to use.  New code
// should not call these functions directly; use the unqualified functions,
// above, instead.

EXPCL_PANDA_LINMATH bool
decompose_matrix_old_hpr(const FLOATNAME(LMatrix3) &mat,
                         FLOATNAME(LVecBase3) &scale,
                         FLOATNAME(LVecBase3) &shear,
                         FLOATNAME(LVecBase3) &hpr,
                         CoordinateSystem cs = CS_default);

EXPCL_PANDA_LINMATH FLOATNAME(LVecBase3)
old_to_new_hpr(const FLOATNAME(LVecBase3) &old_hpr);

END_PUBLISH

#include "compose_matrix_src.I"
