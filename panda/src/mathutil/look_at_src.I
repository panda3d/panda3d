// Filename: look_at_src.I
// Created by:  drose (25Sep99)
// 
////////////////////////////////////////////////////////////////////

INLINE_MATHUTIL void
heads_up(FLOATNAME(LMatrix3) &mat, const FLOATNAME(LVector3) &fwd, 
         CoordinateSystem cs) {
  heads_up(mat, fwd, FLOATNAME(LVector3)::up(cs), cs);
}

INLINE_MATHUTIL void
look_at(FLOATNAME(LMatrix3) &mat, const FLOATNAME(LVector3) &fwd, 
        CoordinateSystem cs) {
  look_at(mat, fwd, FLOATNAME(LVector3)::up(cs), cs);
}

INLINE_MATHUTIL void
heads_up(FLOATNAME(LMatrix4) &mat, const FLOATNAME(LVector3) &fwd,
         const FLOATNAME(LVector3) &up, CoordinateSystem cs) {
  FLOATNAME(LMatrix3) mat3;
  heads_up(mat3, fwd, up, cs);
  mat = FLOATNAME(LMatrix4)(mat3);
}

INLINE_MATHUTIL void
look_at(FLOATNAME(LMatrix4) &mat, const FLOATNAME(LVector3) &fwd, 
        const FLOATNAME(LVector3) &up, CoordinateSystem cs) {
  FLOATNAME(LMatrix3) mat3;
  look_at(mat3, fwd, up, cs);
  mat = FLOATNAME(LMatrix4)(mat3);
}

INLINE_MATHUTIL void
heads_up(FLOATNAME(LMatrix4) &mat, const FLOATNAME(LVector3) &fwd, 
         CoordinateSystem cs) {
  heads_up(mat, fwd, FLOATNAME(LVector3)::up(cs), cs);
}

INLINE_MATHUTIL void
look_at(FLOATNAME(LMatrix4) &mat, const FLOATNAME(LVector3) &fwd, 
        CoordinateSystem cs) {
  look_at(mat, fwd, FLOATNAME(LVector3)::up(cs), cs);
}
  
