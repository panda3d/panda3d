////////////////////////////////////////////////////////////////////
// Filename: compose_matrix.I
// Created by:  drose (21Feb99)
// 
////////////////////////////////////////////////////////////////////

BEGIN_PUBLISH

EXPCL_PANDA INLINE void
compose_matrix(FLOATNAME(LMatrix3) &mat,
	       const FLOATNAME(LVecBase3) &scale,
	       const FLOATNAME(LVecBase3) &hpr,
	       CoordinateSystem cs = CS_default);

EXPCL_PANDA INLINE void
compose_matrix(FLOATNAME(LMatrix4) &mat,
	       const FLOATNAME(LVecBase3) &scale,
	       const FLOATNAME(LVecBase3) &hpr,
	       const FLOATNAME(LVecBase3) &translate,
	       CoordinateSystem cs = CS_default);

INLINE void 
compose_matrix(FLOATNAME(LMatrix4) &mat, const float components[9],
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
		 float roll,
		 CoordinateSystem cs = CS_default);

EXPCL_PANDA bool
decompose_matrix(const FLOATNAME(LMatrix4) &mat,
		 FLOATNAME(LVecBase3) &scale,
		 FLOATNAME(LVecBase3) &hpr,
		 FLOATNAME(LVecBase3) &translate,
		 CoordinateSystem cs = CS_default);

EXPCL_PANDA bool
decompose_matrix(const FLOATNAME(LMatrix4) &mat,
		 FLOATNAME(LVecBase3) &scale,
		 FLOATNAME(LVecBase3) &hpr,
		 FLOATNAME(LVecBase3) &translate,
		 float roll,
		 CoordinateSystem cs = CS_default);

INLINE bool 
decompose_matrix(const FLOATNAME(LMatrix4) &mat, float components[9],
			     CoordinateSystem CS = CS_default);

END_PUBLISH


////////////////////////////////////////////////////////////////////
//     Function: compose_matrix
//  Description: Computes the 3x3 matrix from scale and rotation.
////////////////////////////////////////////////////////////////////

INLINE void
_compose_matrix(FLOATNAME(LMatrix3) &mat,
		const FLOATNAME(LVecBase3) &scale,
		const FLOATNAME(LVecBase3) &hpr,
		CoordinateSystem cs) {
  mat =
    FLOATNAME(LMatrix3)::scale_mat(scale) *
    FLOATNAME(LMatrix3)::rotate_mat(hpr[1], FLOATNAME(LVector3)::right(cs), cs) *
    FLOATNAME(LMatrix3)::rotate_mat(hpr[0], FLOATNAME(LVector3)::up(cs), cs) *
    FLOATNAME(LMatrix3)::rotate_mat(hpr[2], FLOATNAME(LVector3)::back(cs), cs);
}

////////////////////////////////////////////////////////////////////
//     Function: compose_matrix
//  Description: Computes the 4x4 matrix according to scale, rotation,
//               and translation.
////////////////////////////////////////////////////////////////////

INLINE void
_compose_matrix(FLOATNAME(LMatrix4) &mat,
		const FLOATNAME(LVecBase3) &scale,
		const FLOATNAME(LVecBase3) &hpr,
		const FLOATNAME(LVecBase3) &translate,
		CoordinateSystem cs) {
  FLOATNAME(LMatrix3) upper3;
  _compose_matrix(upper3, scale, hpr, cs);
  mat = FLOATNAME(LMatrix4)(upper3, translate);
}

INLINE void
compose_matrix(FLOATNAME(LMatrix4) &mat,
	       const FLOATTYPE1 components[9],
	       CoordinateSystem cs) {
  FLOATNAME(LVector3) scale(components[0],
		  components[1],
		  components[2]);
  FLOATNAME(LVector3) hpr(components[3],
		components[4],
		components[5]);
  FLOATNAME(LVector3) translate(components[6],
		      components[7],
		      components[8]);
  _compose_matrix(mat, scale, hpr, translate, cs);
}

EXPCL_PANDA INLINE void
compose_matrix(FLOATNAME(LMatrix3) &mat,
	       const FLOATNAME(LVecBase3) &scale,
	       const FLOATNAME(LVecBase3) &hpr,
	       CoordinateSystem cs) {
  _compose_matrix(mat, scale, hpr, cs);
}

EXPCL_PANDA INLINE void
compose_matrix(FLOATNAME(LMatrix4) &mat,
	       const FLOATNAME(LVecBase3) &scale,
	       const FLOATNAME(LVecBase3) &hpr,
	       const FLOATNAME(LVecBase3) &translate,
	       CoordinateSystem cs) {
  _compose_matrix(mat, scale, hpr, translate, cs);
}

