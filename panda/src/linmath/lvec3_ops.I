// Filename: lvec3_ops.I
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

// When possible, operators have been defined within the classes.
// This file defines operator functions outside of classes where
// necessary.  It also defines some convenient out-of-class wrappers
// around in-class functions (like dot, length, normalize).


// scalar * vec (vec * scalar is defined in class)

INLINE FLOATNAME(LVecBase3) 
operator * (FLOATTYPE2 scalar, const FLOATNAME(LVecBase3) &a);


INLINE FLOATNAME(LPoint3) 
operator * (FLOATTYPE2 scalar, const FLOATNAME(LPoint3) &a);


INLINE FLOATNAME(LVector3) 
operator * (FLOATTYPE2 scalar, const FLOATNAME(LVector3) &a);


// dot product

INLINE FLOATTYPE1
dot(const FLOATNAME(LVecBase3) &a, const FLOATNAME(LVecBase3) &b);


// cross product

INLINE FLOATNAME(LVecBase3)
cross(const FLOATNAME(LVecBase3) &a, const FLOATNAME(LVecBase3) &b);


INLINE FLOATNAME(LVector3)
cross(const FLOATNAME(LVector3) &a, const FLOATNAME(LVector3) &b);


// Length of a vector.

INLINE FLOATTYPE1
length(const FLOATNAME(LVector3) &a);


// A normalized vector.

INLINE FLOATNAME(LVector3)
normalize(const FLOATNAME(LVector3) &v);


////////////////////////////////////////////////////////////////////
//     Function: scalar * LVecBase3
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase3) 
operator * (FLOATTYPE2 scalar, const FLOATNAME(LVecBase3) &a) {
  return a * scalar;
}

////////////////////////////////////////////////////////////////////
//     Function: scalar * LPoint3
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LPoint3) 
operator * (FLOATTYPE2 scalar, const FLOATNAME(LPoint3) &a) {
  return a * scalar;
}

////////////////////////////////////////////////////////////////////
//     Function: scalar * LVector3
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVector3) 
operator * (FLOATTYPE2 scalar, const FLOATNAME(LVector3) &a) {
  return a * scalar;
}

////////////////////////////////////////////////////////////////////
//     Function: dot product of LVecBase3
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATTYPE1
dot(const FLOATNAME(LVecBase3) &a, const FLOATNAME(LVecBase3) &b) {
  return a.dot(b);
}

////////////////////////////////////////////////////////////////////
//     Function: cross product of LVecBase3
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase3)
cross(const FLOATNAME(LVecBase3) &a, const FLOATNAME(LVecBase3) &b) {
  return a.cross(b);
}

////////////////////////////////////////////////////////////////////
//     Function: cross product of LVector3
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector3)
cross(const FLOATNAME(LVector3) &a, const FLOATNAME(LVector3) &b) {
  return FLOATNAME(LVector3)(a.cross(b));
}


////////////////////////////////////////////////////////////////////
//     Function: length of a vector
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATTYPE1
length(const FLOATNAME(LVector3) &a) {
  return a.length();
}


////////////////////////////////////////////////////////////////////
//     Function: normalize
//  Description: Returns a normalized vector from the given vector.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector3)
normalize(const FLOATNAME(LVector3) &v) {
  FLOATNAME(LVector3) v1 = v;
  v1.normalize();
  return v1;
}
