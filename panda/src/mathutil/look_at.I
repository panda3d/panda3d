// Filename: look_at.I
// Created by:  drose (25Sep99)
// 
////////////////////////////////////////////////////////////////////

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

BEGIN_PUBLISH

// Flavors for float-type arithmetic.

EXPCL_PANDA void
heads_up(FLOATNAME(LMatrix3) &mat, const FLOATNAME(LVector3) &fwd,
	 const FLOATNAME(LVector3) &up = FLOATNAME(LVector3)::up(),
	 CoordinateSystem cs = CS_default);
EXPCL_PANDA void
look_at(FLOATNAME(LMatrix3) &mat, const FLOATNAME(LVector3) &fwd,
	const FLOATNAME(LVector3) &up = FLOATNAME(LVector3)::up(),
	CoordinateSystem cs = CS_default);

INLINE void heads_up(FLOATNAME(LMatrix3) &mat, const FLOATNAME(LVector3) &fwd,
		     CoordinateSystem cs);
INLINE void look_at(FLOATNAME(LMatrix3) &mat, const FLOATNAME(LVector3) &fwd,
		    CoordinateSystem cs);


INLINE void heads_up(FLOATNAME(LMatrix4) &mat, const FLOATNAME(LVector3) &fwd,
		     const FLOATNAME(LVector3) &up = FLOATNAME(LVector3)::up(),
		     CoordinateSystem cs = CS_default);
INLINE void look_at(FLOATNAME(LMatrix4) &mat, const FLOATNAME(LVector3) &fwd,
		    const FLOATNAME(LVector3) &up = FLOATNAME(LVector3)::up(),
		    CoordinateSystem cs = CS_default);

INLINE void heads_up(FLOATNAME(LMatrix4) &mat, const FLOATNAME(LVector3) &fwd,
		     CoordinateSystem cs);
INLINE void look_at(FLOATNAME(LMatrix4) &mat, const FLOATNAME(LVector3) &fwd,
		    CoordinateSystem cs);

END_PUBLISH


INLINE void
heads_up(FLOATNAME(LMatrix3) &mat, const FLOATNAME(LVector3) &fwd, CoordinateSystem cs) {
  heads_up(mat, fwd, FLOATNAME(LVector3)::up(cs), cs);
}

INLINE void
look_at(FLOATNAME(LMatrix3) &mat, const FLOATNAME(LVector3) &fwd, CoordinateSystem cs) {
  look_at(mat, fwd, FLOATNAME(LVector3)::up(cs), cs);
}

INLINE void
heads_up(FLOATNAME(LMatrix4) &mat, const FLOATNAME(LVector3) &fwd,
	 const FLOATNAME(LVector3) &up, CoordinateSystem cs) {
  FLOATNAME(LMatrix3) mat3;
  heads_up(mat3, fwd, up, cs);
  mat = FLOATNAME(LMatrix4)(mat3);
}

INLINE void
look_at(FLOATNAME(LMatrix4) &mat, const FLOATNAME(LVector3) &fwd, 
	const FLOATNAME(LVector3) &up, CoordinateSystem cs) {
  FLOATNAME(LMatrix3) mat3;
  look_at(mat3, fwd, up, cs);
  mat = FLOATNAME(LMatrix4)(mat3);
}

INLINE void
heads_up(FLOATNAME(LMatrix4) &mat, const FLOATNAME(LVector3) &fwd, CoordinateSystem cs) {
  heads_up(mat, fwd, FLOATNAME(LVector3)::up(cs), cs);
}

INLINE void
look_at(FLOATNAME(LMatrix4) &mat, const FLOATNAME(LVector3) &fwd, CoordinateSystem cs) {
  look_at(mat, fwd, FLOATNAME(LVector3)::up(cs), cs);
}
  
