// Filename: look_at.I
// Created by:  drose (25Sep99)
// 
////////////////////////////////////////////////////////////////////

INLINE void
heads_up(LMatrix3f &mat, const LVector3f &fwd, CoordinateSystem cs) {
  heads_up(mat, fwd, LVector3f::up(cs), cs);
}

INLINE void
look_at(LMatrix3f &mat, const LVector3f &fwd, CoordinateSystem cs) {
  look_at(mat, fwd, LVector3f::up(cs), cs);
}

INLINE void
heads_up(LMatrix4f &mat, const LVector3f &fwd,
	 const LVector3f &up, CoordinateSystem cs) {
  LMatrix3f mat3;
  heads_up(mat3, fwd, up, cs);
  mat = LMatrix4f(mat3);
}

INLINE void
look_at(LMatrix4f &mat, const LVector3f &fwd, 
	const LVector3f &up, CoordinateSystem cs) {
  LMatrix3f mat3;
  look_at(mat3, fwd, up, cs);
  mat = LMatrix4f(mat3);
}

INLINE void
heads_up(LMatrix4f &mat, const LVector3f &fwd, CoordinateSystem cs) {
  heads_up(mat, fwd, LVector3f::up(cs), cs);
}

INLINE void
look_at(LMatrix4f &mat, const LVector3f &fwd, CoordinateSystem cs) {
  look_at(mat, fwd, LVector3f::up(cs), cs);
}
  


INLINE void
heads_up(LMatrix3d &mat, const LVector3d &fwd, CoordinateSystem cs) {
  heads_up(mat, fwd, LVector3d::up(cs), cs);
}

INLINE void
look_at(LMatrix3d &mat, const LVector3d &fwd, CoordinateSystem cs) {
  look_at(mat, fwd, LVector3d::up(cs), cs);
}

INLINE void
heads_up(LMatrix4d &mat, const LVector3d &fwd,
	 const LVector3d &up, CoordinateSystem cs) {
  LMatrix3d mat3;
  heads_up(mat3, fwd, up, cs);
  mat = LMatrix4d(mat3);
}

INLINE void
look_at(LMatrix4d &mat, const LVector3d &fwd, 
	const LVector3d &up, CoordinateSystem cs) {
  LMatrix3d mat3;
  look_at(mat3, fwd, up, cs);
  mat = LMatrix4d(mat3);
}

INLINE void
heads_up(LMatrix4d &mat, const LVector3d &fwd, CoordinateSystem cs) {
  heads_up(mat, fwd, LVector3d::up(cs), cs);
}

INLINE void
look_at(LMatrix4d &mat, const LVector3d &fwd, CoordinateSystem cs) {
  look_at(mat, fwd, LVector3d::up(cs), cs);
}
  
