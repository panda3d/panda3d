// Filename: look_at.h
// Created by:  drose (25Apr97)
//
////////////////////////////////////////////////////////////////////

#ifndef LOOKAT_H
#define LOOKAT_H

////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <luse.h>
#include <coordinateSystem.h>

// These functions return a matrix that rotates between a coordinate
// system defined with the given forward and up vectors, and the
// standard coordinate system with y-forward and z-up.  They differ
// only in their behavior when the supplied forward and up vectors are
// not perpendicular; in this case, look_at will match the forward
// vector precisely, while heads_up will match the up vector
// precisely.

// Since these functions only return a rotation matrix, the
// translation component is always zero.  There are flavors of these
// functions that simply return the upper 3x3 part of the matrix, and
// flavors that return the whole 4x4 matrix with a zero bottom row.


// Flavors for float-type arithmetic.

EXPCL_PANDA void
heads_up(LMatrix3f &mat, const LVector3f &fwd,
	 const LVector3f &up = LVector3f::up(),
	 CoordinateSystem cs = CS_default);
EXPCL_PANDA void
look_at(LMatrix3f &mat, const LVector3f &fwd,
	const LVector3f &up = LVector3f::up(),
	CoordinateSystem cs = CS_default);

INLINE void heads_up(LMatrix3f &mat, const LVector3f &fwd,
		     CoordinateSystem cs);
INLINE void look_at(LMatrix3f &mat, const LVector3f &fwd,
		    CoordinateSystem cs);


INLINE void heads_up(LMatrix4f &mat, const LVector3f &fwd,
		     const LVector3f &up = LVector3f::up(),
		     CoordinateSystem cs = CS_default);
INLINE void look_at(LMatrix4f &mat, const LVector3f &fwd,
		    const LVector3f &up = LVector3f::up(),
		    CoordinateSystem cs = CS_default);

INLINE void heads_up(LMatrix4f &mat, const LVector3f &fwd,
		     CoordinateSystem cs);
INLINE void look_at(LMatrix4f &mat, const LVector3f &fwd,
		    CoordinateSystem cs);


// Flavors for double-type arithmetic.

EXPCL_PANDA void
heads_up(LMatrix3d &mat, const LVector3d &fwd,
	 const LVector3d &up = LVector3d::up(),
	 CoordinateSystem cs = CS_default);
EXPCL_PANDA void
look_at(LMatrix3d &mat, const LVector3d &fwd,
	const LVector3d &up = LVector3d::up(),
	CoordinateSystem cs = CS_default);

INLINE void heads_up(LMatrix3d &mat, const LVector3d &fwd,
		     CoordinateSystem cs);
INLINE void look_at(LMatrix3d &mat, const LVector3d &fwd,
		    CoordinateSystem cs);


INLINE void heads_up(LMatrix4d &mat, const LVector3d &fwd,
		     const LVector3d &up = LVector3d::up(),
		     CoordinateSystem cs = CS_default);
INLINE void look_at(LMatrix4d &mat, const LVector3d &fwd,
		    const LVector3d &up = LVector3d::up(),
		    CoordinateSystem cs = CS_default);

INLINE void heads_up(LMatrix4d &mat, const LVector3d &fwd,
		     CoordinateSystem cs);
INLINE void look_at(LMatrix4d &mat, const LVector3d &fwd,
		    CoordinateSystem cs);

#include "look_at.I"

#endif
