// Filename: lmat_ops_src.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: LVecBase3 times LMatrix3
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE_LINMATH FLOATNAME(LVecBase3)
operator * (const FLOATNAME(LVecBase3) &v, const FLOATNAME(LMatrix3) &m) {
  return m.xform(v);
}

INLINE_LINMATH FLOATNAME(LVector3)
operator * (const FLOATNAME(LVector3) &v, const FLOATNAME(LMatrix3) &m) {
  return m.xform(v);
}

INLINE_LINMATH FLOATNAME(LPoint3)
operator * (const FLOATNAME(LPoint3) &v, const FLOATNAME(LMatrix3) &m) {
  return m.xform(v);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2 times LMatrix3
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE_LINMATH FLOATNAME(LVector2)
operator * (const FLOATNAME(LVector2) &v, const FLOATNAME(LMatrix3) &m) {
  return m.xform_vec(v);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2 times LMatrix3
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE_LINMATH FLOATNAME(LPoint2)
operator * (const FLOATNAME(LPoint2) &v, const FLOATNAME(LMatrix3) &m) {
  return m.xform_point(v);
}


////////////////////////////////////////////////////////////////////
//     Function: LVecBase4 times LMatrix4
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE_LINMATH FLOATNAME(LVecBase4)
operator * (const FLOATNAME(LVecBase4) &v, const FLOATNAME(LMatrix4) &m) {
  return m.xform(v);
}

INLINE_LINMATH FLOATNAME(LVector4)
operator * (const FLOATNAME(LVector4) &v, const FLOATNAME(LMatrix4) &m) {
  return m.xform(v);
}

INLINE_LINMATH FLOATNAME(LPoint4)
operator * (const FLOATNAME(LPoint4) &v, const FLOATNAME(LMatrix4) &m) {
  return m.xform(v);
}


////////////////////////////////////////////////////////////////////
//     Function: LVector3 times LMatrix4
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE_LINMATH FLOATNAME(LVector3)
operator * (const FLOATNAME(LVector3) &v, const FLOATNAME(LMatrix4) &m) {
  return m.xform_vec(v);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3 times LMatrix4
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE_LINMATH FLOATNAME(LPoint3)
operator * (const FLOATNAME(LPoint3) &v, const FLOATNAME(LMatrix4) &m) {
  return m.xform_point(v);
}

