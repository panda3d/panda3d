// Filename: lvec4_ops.I
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

// When possible, operators have been defined within the classes.
// This file defines operator functions outside of classes where
// necessary.  It also defines some convenient out-of-class wrappers
// around in-class functions (like dot, length, normalize).


// scalar * vec (vec * scalar is defined in class)

INLINE FLOATNAME(LVecBase4) 
operator * (FLOATTYPE2 scalar, const FLOATNAME(LVecBase4) &a);


INLINE FLOATNAME(LPoint4) 
operator * (FLOATTYPE2 scalar, const FLOATNAME(LPoint4) &a);


INLINE FLOATNAME(LVector4) 
operator * (FLOATTYPE2 scalar, const FLOATNAME(LVector4) &a);


// dot product

INLINE FLOATTYPE1
dot(const FLOATNAME(LVecBase4) &a, const FLOATNAME(LVecBase4) &b);


// Length of a vector.

INLINE FLOATTYPE1
length(const FLOATNAME(LVector4) &a);


// A normalized vector.

INLINE FLOATNAME(LVector4)
normalize(const FLOATNAME(LVector4) &v);


////////////////////////////////////////////////////////////////////
//     Function: scalar * LVecBase4
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase4) 
operator * (FLOATTYPE2 scalar, const FLOATNAME(LVecBase4) &a) {
  return a * scalar;
}

////////////////////////////////////////////////////////////////////
//     Function: scalar * LPoint4
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LPoint4) 
operator * (FLOATTYPE2 scalar, const FLOATNAME(LPoint4) &a) {
  return a * scalar;
}

////////////////////////////////////////////////////////////////////
//     Function: scalar * LVector4
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVector4) 
operator * (FLOATTYPE2 scalar, const FLOATNAME(LVector4) &a) {
  return a * scalar;
}

////////////////////////////////////////////////////////////////////
//     Function: dot product of LVecBase4
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATTYPE1
dot(const FLOATNAME(LVecBase4) &a, const FLOATNAME(LVecBase4) &b) {
  return a.dot(b);
}

////////////////////////////////////////////////////////////////////
//     Function: length of a vector
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATTYPE1
length(const FLOATNAME(LVector4) &a) {
  return a.length();
}


////////////////////////////////////////////////////////////////////
//     Function: normalize
//  Description: Returns a normalized vector from the given vector.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector4)
normalize(const FLOATNAME(LVector4) &v) {
  FLOATNAME(LVector4) v1 = v;
  v1.normalize();
  return v1;
}
