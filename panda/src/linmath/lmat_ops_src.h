// Filename: lmat_ops_src.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

BEGIN_PUBLISH

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

END_PUBLISH

#include "lmat_ops_src.I"
