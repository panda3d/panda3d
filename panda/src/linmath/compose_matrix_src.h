////////////////////////////////////////////////////////////////////
// Filename: compose_matrix_src.h
// Created by:  drose (21Feb99)
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
                 CoordinateSystem cs = CS_default);

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
                 CoordinateSystem cs = CS_default);

INLINE_LINMATH bool 
decompose_matrix(const FLOATNAME(LMatrix4) &mat, FLOATTYPE components[9],
                 CoordinateSystem CS = CS_default);

END_PUBLISH

#include "compose_matrix_src.I"
