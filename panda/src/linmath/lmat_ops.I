// Filename: lmat_ops.I
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

/*
// vector times matrix3

INLINE FLOATNAME(LVecBase3)
operator * (const FLOATNAME(LVecBase3) &v, const FLOATNAME(LMatrix3) &m);


INLINE FLOATNAME(LVector2)
operator * (const FLOATNAME(LVector2) &v, const FLOATNAME(LMatrix3) &m);


INLINE FLOATNAME(LPoint2)
operator * (const FLOATNAME(LPoint2) &v, const FLOATNAME(LMatrix3) &m);


// vector times matrix4

INLINE FLOATNAME(LVecBase4)
operator * (const FLOATNAME(LVecBase4) &v, const FLOATNAME(LMatrix4) &m);


INLINE FLOATNAME(LVector3)
operator * (const FLOATNAME(LVector3) &v, const FLOATNAME(LMatrix4) &m);


INLINE FLOATNAME(LPoint3)
operator * (const FLOATNAME(LPoint3) &v, const FLOATNAME(LMatrix4) &m);
*/
#ifdef CPPPARSER
// Strictly for the benefit of interrogate, we'll define explicit
// 'instantiations' of the above template functions on types float and
// double.

BEGIN_PUBLISH

INLINE FLOATNAME(LVecBase3)
operator * (const FLOATNAME(LVecBase3) &v, const FLOATNAME(LMatrix3) &m);
INLINE FLOATNAME(LVector2)
operator * (const FLOATNAME(LVector2) &v, const FLOATNAME(LMatrix3) &m);
INLINE FLOATNAME(LPoint2)
operator * (const FLOATNAME(LPoint2) &v, const FLOATNAME(LMatrix3) &m);
INLINE FLOATNAME(LVecBase4)
operator * (const FLOATNAME(LVecBase4) &v, const FLOATNAME(LMatrix4) &m);
INLINE FLOATNAME(LVector3)
operator * (const FLOATNAME(LVector3) &v, const FLOATNAME(LMatrix4) &m);
INLINE FLOATNAME(LPoint3)
operator * (const FLOATNAME(LPoint3) &v, const FLOATNAME(LMatrix4) &m);

/*
INLINE LVecBase3<double>
operator * (const LVecBase3<double> &v, const LMatrix3<double> &m);
INLINE LVector2<double>
operator * (const LVector2<double> &v, const LMatrix3<double> &m);
INLINE LPoint2<double>
operator * (const LPoint2<double> &v, const LMatrix3<double> &m);
INLINE LVecBase4<double>
operator * (const LVecBase4<double> &v, const LMatrix4<double> &m);
INLINE LVector3<double>
operator * (const LVector3<double> &v, const LMatrix4<double> &m);
INLINE LPoint3<double>
operator * (const LPoint3<double> &v, const LMatrix4<double> &m);
*/

END_PUBLISH

#endif  // CPPPARSER

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3 times LMatrix3
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase3)
operator * (const FLOATNAME(LVecBase3) &v, const FLOATNAME(LMatrix3) &m) {
  return m.xform(v);
}

INLINE FLOATNAME(LVector3)
operator * (const FLOATNAME(LVector3) &v, const FLOATNAME(LMatrix3) &m) {
  return m.xform(v);
}

INLINE FLOATNAME(LPoint3)
operator * (const FLOATNAME(LPoint3) &v, const FLOATNAME(LMatrix3) &m) {
  return m.xform(v);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2 times LMatrix3
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector2)
operator * (const FLOATNAME(LVector2) &v, const FLOATNAME(LMatrix3) &m) {
  return m.xform_vec(v);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2 times LMatrix3
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint2)
operator * (const FLOATNAME(LPoint2) &v, const FLOATNAME(LMatrix3) &m) {
  return m.xform_point(v);
}


////////////////////////////////////////////////////////////////////
//     Function: LVecBase4 times LMatrix4
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase4)
operator * (const FLOATNAME(LVecBase4) &v, const FLOATNAME(LMatrix4) &m) {
  return m.xform(v);
}

INLINE FLOATNAME(LVector4)
operator * (const FLOATNAME(LVector4) &v, const FLOATNAME(LMatrix4) &m) {
  return m.xform(v);
}

INLINE FLOATNAME(LPoint4)
operator * (const FLOATNAME(LPoint4) &v, const FLOATNAME(LMatrix4) &m) {
  return m.xform(v);
}


////////////////////////////////////////////////////////////////////
//     Function: LVector3 times LMatrix4
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector3)
operator * (const FLOATNAME(LVector3) &v, const FLOATNAME(LMatrix4) &m) {
  return m.xform_vec(v);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3 times LMatrix4
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LPoint3)
operator * (const FLOATNAME(LPoint3) &v, const FLOATNAME(LMatrix4) &m) {
  return m.xform_point(v);
}

