// Filename: lvec2_ops.I
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

// When possible, operators have been defined within the classes.
// This file defines operator functions outside of classes where
// necessary.  It also defines some convenient out-of-class wrappers
// around in-class functions (like dot, length, normalize).


// scalar * vec (vec * scalar is defined in class)

INLINE FLOATNAME(LVecBase2) 
operator * (FLOATTYPE2 scalar, const FLOATNAME(LVecBase2) &a);


INLINE FLOATNAME(LPoint2) 
operator * (FLOATTYPE2 scalar, const FLOATNAME(LPoint2) &a);


INLINE FLOATNAME(LVector2) 
operator * (FLOATTYPE2 scalar, const FLOATNAME(LVector2) &a);


// dot product

INLINE FLOATTYPE1
dot(const FLOATNAME(LVecBase2) &a, const FLOATNAME(LVecBase2) &b);


// Length of a vector.

INLINE FLOATTYPE1
length(const FLOATNAME(LVector2) &a);


// A normalized vector.

INLINE FLOATNAME(LVector2)
normalize(const FLOATNAME(LVector2) &v);

////////////////////////////////////////////////////////////////////
//     Function: scalar * LVecBase2
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase2) 
operator * (FLOATTYPE2 scalar, const FLOATNAME(LVecBase2) &a) {
  return a * scalar;
}

////////////////////////////////////////////////////////////////////
//     Function: scalar * LPoint2
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LPoint2) 
operator * (FLOATTYPE2 scalar, const FLOATNAME(LPoint2) &a) {
  return a * scalar;
}

////////////////////////////////////////////////////////////////////
//     Function: scalar * LVector2
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVector2) 
operator * (FLOATTYPE2 scalar, const FLOATNAME(LVector2) &a) {
  return a * scalar;
}

////////////////////////////////////////////////////////////////////
//     Function: dot product of LVecBase2
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATTYPE1
dot(const FLOATNAME(LVecBase2) &a, const FLOATNAME(LVecBase2) &b) {
  return a.dot(b);
}

////////////////////////////////////////////////////////////////////
//     Function: length of a vector
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATTYPE1
length(const FLOATNAME(LVector2) &a) {
  return a.length();
}


////////////////////////////////////////////////////////////////////
//     Function: normalize
//  Description: Returns a normalized vector from the given vector.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector2)
normalize(const FLOATNAME(LVector2) &v) {
  FLOATNAME(LVector2) v1 = v;
  v1.normalize();
  return v1;
}
