// Filename: compose_matrix.h
// Created by:  drose (27Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef COMPOSE_MATRIX_H
#define COMPOSE_MATRIX_H

////////////////////////////////////////////////////////////////////
//
// compose_matrix(), decompose_matrix()
//
// These two functions build and/or extract an affine matrix into
// its constituent parts: scale, hpr, and translate.
//
// The functions here come in two flavors, for doubles and floats.
// They're actually implemented as template functions so could be
// instantiated on any numeric type, but I only bothered to
// instantiate and expose these two.  It saves on some cross-library
// pre-linking stuff to have all the templates be hidden.
//
// There are also two additional flavors for 3x3 matrices.  These are
// treated as the upper 3x3 part of a general 4x4 matrix, and so can
// only represent rotations and scales.
//
////////////////////////////////////////////////////////////////////

#include <pandabase.h>

#include "lmatrix.h"
#include "luse.h"

BEGIN_PUBLISH

EXPCL_PANDA void
compose_matrix(LMatrix3f &mat,
	       const LVecBase3f &scale,
	       const LVecBase3f &hpr,
	       CoordinateSystem cs = CS_default);
EXPCL_PANDA bool
decompose_matrix(const LMatrix3f &mat,
		 LVecBase3f &scale,
		 LVecBase3f &hpr,
		 CoordinateSystem cs = CS_default);
EXPCL_PANDA bool
decompose_matrix(const LMatrix3f &mat,
		 LVecBase3f &scale,
		 LVecBase3f &hpr,
		 float roll,
		 CoordinateSystem cs = CS_default);

EXPCL_PANDA void
compose_matrix(LMatrix3d &mat,
	       const LVecBase3d &scale,
	       const LVecBase3d &hpr,
	       CoordinateSystem cs = CS_default);
EXPCL_PANDA bool
decompose_matrix(const LMatrix3d &mat,
		 LVecBase3d &scale,
		 LVecBase3d &hpr,
		 CoordinateSystem cs = CS_default);
EXPCL_PANDA bool
decompose_matrix(const LMatrix3d &mat,
		 LVecBase3d &scale,
		 LVecBase3d &hpr,
		 double roll,
		 CoordinateSystem cs = CS_default);

EXPCL_PANDA void
compose_matrix(LMatrix4f &mat,
	       const LVecBase3f &scale,
	       const LVecBase3f &hpr,
	       const LVecBase3f &translate,
	       CoordinateSystem cs = CS_default);
INLINE void compose_matrix(LMatrix4f &mat, const float components[9],
			   CoordinateSystem cs = CS_default);

EXPCL_PANDA bool
decompose_matrix(const LMatrix4f &mat,
		 LVecBase3f &scale,
		 LVecBase3f &hpr,
		 LVecBase3f &translate,
		 CoordinateSystem cs = CS_default);
EXPCL_PANDA bool
decompose_matrix(const LMatrix4f &mat,
		 LVecBase3f &scale,
		 LVecBase3f &hpr,
		 LVecBase3f &translate,
		 float roll,
		 CoordinateSystem cs = CS_default);
INLINE bool decompose_matrix(const LMatrix4f &mat, float components[9],
			     CoordinateSystem CS = CS_default);

EXPCL_PANDA void
compose_matrix(LMatrix4d &mat,
	       const LVecBase3d &scale,
	       const LVecBase3d &hpr,
	       const LVecBase3d &translate,
	       CoordinateSystem cs = CS_default);
INLINE void compose_matrix(LMatrix4d &mat, const double components[9],
			   CoordinateSystem cs = CS_default);

bool EXPCL_PANDA
decompose_matrix(const LMatrix4d &mat,
		 LVecBase3d &scale,
		 LVecBase3d &hpr,
		 LVecBase3d &translate,
		 CoordinateSystem cs = CS_default);
bool EXPCL_PANDA
decompose_matrix(const LMatrix4d &mat,
		 LVecBase3d &scale,
		 LVecBase3d &hpr,
		 LVecBase3d &translate,
		 double roll,
		 CoordinateSystem cs = CS_default);
INLINE bool decompose_matrix(const LMatrix4d &mat, double components[9],
			     CoordinateSystem cs = CS_default);

END_PUBLISH

#include "compose_matrix.I"

#endif

