// Filename: lmat_ops.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LMAT_OPS_H
#define LMAT_OPS_H

#include "lvecBase3.h"
#include "lpoint3.h"
#include "lvector3.h"
#include "lvecBase4.h"
#include "lpoint4.h"
#include "lvector4.h"
#include "lmatrix3.h"
#include "lmatrix4.h"


// vector times matrix3
template<class NumType>
INLINE LVecBase3<NumType>
operator * (const LVecBase3<NumType> &v, const LMatrix3<NumType> &m);

template<class NumType>
INLINE LVector2<NumType>
operator * (const LVector2<NumType> &v, const LMatrix3<NumType> &m);

template<class NumType>
INLINE LPoint2<NumType>
operator * (const LPoint2<NumType> &v, const LMatrix3<NumType> &m);


// vector times matrix4
template<class NumType>
INLINE LVecBase4<NumType>
operator * (const LVecBase4<NumType> &v, const LMatrix4<NumType> &m);

template<class NumType>
INLINE LVector3<NumType>
operator * (const LVector3<NumType> &v, const LMatrix4<NumType> &m);

template<class NumType>
INLINE LPoint3<NumType>
operator * (const LPoint3<NumType> &v, const LMatrix4<NumType> &m);

#ifdef CPPPARSER
// Strictly for the benefit of interrogate, we'll define explicit
// 'instantiations' of the above template functions on types float and
// double.

BEGIN_PUBLISH

INLINE LVecBase3<float>
operator * (const LVecBase3<float> &v, const LMatrix3<float> &m);
INLINE LVector2<float>
operator * (const LVector2<float> &v, const LMatrix3<float> &m);
INLINE LPoint2<float>
operator * (const LPoint2<float> &v, const LMatrix3<float> &m);
INLINE LVecBase4<float>
operator * (const LVecBase4<float> &v, const LMatrix4<float> &m);
INLINE LVector3<float>
operator * (const LVector3<float> &v, const LMatrix4<float> &m);
INLINE LPoint3<float>
operator * (const LPoint3<float> &v, const LMatrix4<float> &m);

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

END_PUBLISH

#endif  // CPPPARSER

#include "lmat_ops.I"

#endif
