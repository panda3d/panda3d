// Filename: luse.h
// Created by:  drose (13Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef LUSE_H
#define LUSE_H

////////////////////////////////////////////////////////////////////
//
// This file defines a number of vector-based classes that are
// designed for specific uses.  These all inherit from LVecBase, which
// is the base class of all linear algebra vectors.
//
// LPoint<P_FLOATTYPE1, N_length>
//
//   This should be used to represent a specific point in space.  It
//   inherits most properties from LVecBase.
//
// LVector<P_FLOATTYPE1, N_length>
//
//   This should be used to represent a vector, or a distance between
//   two points in space.
//
// The distinction between LPoint and LVector is worth emphasizing.
// They differ in some subtle typing behavior (vector - vector =
// vector, point + vector = point, point - point = vector) and also in
// the way they are transformed when multiplied by a matrix (a point
// gets the translation component of the matrix, while the vector does
// not).  Also, vector has length() and normalize() functions defined
// for it, while point does not.
//
// LPoint and LVector should be used whenever the concept of "point"
// or "vector" applies.  If neither applies--for instance, if you are
// storing a plane equation in a vector or some such nonsense--use the
// base class, LVecBase.
//
// This file also defines the following:
//
// Vertex<P_FLOATTYPE1>, Vertexd, Vertexf
// Normal<P_FLOATTYPE1>, Normald, Normalf
// TexCoord<P_FLOATTYPE1>, TexCoordd, TexCoordf
// Color<P_FLOATTYPE1>, Colord, Colorf
// RGBColor<P_FLOATTYPE1>, RGBColord, RGBColorf
//
// These classes are derivations of LPoint or LVector, as appropriate,
// and are intended to store a specific kind of rendering attribute.
// (Color is a four-component color; RGBColor is three-component.)
//
////////////////////////////////////////////////////////////////////

#include <pandabase.h>

#include "lvec2_ops.h"
#include "lvec3_ops.h"
#include "lvec4_ops.h"
#include "lmat_ops.h"
#include "lmatrix.h"
#include "lquaternion.h"
#include "lrotation.h"
#include "lorientation.h"

// This macro defines the cast-to-another-numeric-type operator for
// all of the things defined in this package.  It works by virtue of
// there being an appropriate lcast_to() template function defined for
// each class.

#define LCAST(numeric_type, object) lcast_to((numeric_type *)0, object)

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif


// Now we define some handy typedefs for these classes.
/*
typedef LVecBase2<float> LVecBase2f;
typedef LVecBase3<float> LVecBase3f;
typedef LVecBase4<float> LVecBase4f;

typedef LVector2<float> LVector2f;
typedef LVector3<float> LVector3f;
typedef LVector4<float> LVector4f;
typedef LPoint2<float> LPoint2f;
typedef LPoint3<float> LPoint3f;
typedef LPoint4<float> LPoint4f;
*/
typedef LPoint3f Vertexf;
typedef LVector3f Normalf;
typedef LPoint2f TexCoordf;
typedef LVecBase4f Colorf;
typedef LVecBase3f RGBColorf;
/*
typedef LVecBase2<double> LVecBase2d;
typedef LVecBase3<double> LVecBase3d;
typedef LVecBase4<double> LVecBase4d;

typedef LVector2<double> LVector2d;
typedef LVector3<double> LVector3d;
typedef LVector4<double> LVector4d;
typedef LPoint2<double> LPoint2d;
typedef LPoint3<double> LPoint3d;
typedef LPoint4<double> LPoint4d;
*/
typedef LPoint3d Vertexd;
typedef LVector3d Normald;
typedef LPoint2d TexCoordd;
typedef LVecBase4d Colord;
typedef LVecBase3d RGBColord;
typedef LQuaternionBasef LQuaternionf;
typedef LQuaternionBased LQuaterniond;

/*
typedef LQuaternionBase<float> LQuaternionf;
typedef LRotation<float> LRotationf;
typedef LOrientation<float> LOrientationf;

typedef LQuaternionBase<double> LQuaterniond;
typedef LRotation<double> LRotationd;
typedef LOrientation<double> LOrientationd;
*/
/*
// Now define explicit instantiations of the output operator functions
// for interrogate's benefit.  These functions don't actually exist
// anywhere, but interrogate can't know how to instantiate template
// functions, so we pretend these are real functions.
#ifdef CPPPARSER
INLINE ostream &operator << (ostream &out, const LVecBase2f &vec);
INLINE ostream &operator << (ostream &out, const LVecBase3f &vec);
INLINE ostream &operator << (ostream &out, const LVecBase4f &vec);
INLINE ostream &operator << (ostream &out, const LMatrix3f &mat);
INLINE ostream &operator << (ostream &out, const LMatrix4f &mat);
INLINE ostream &operator << (ostream &out, const LQuaternionf &q);

INLINE ostream &operator << (ostream &out, const LVecBase2d &vec);
INLINE ostream &operator << (ostream &out, const LVecBase3d &vec);
INLINE ostream &operator << (ostream &out, const LVecBase4d &vec);
INLINE ostream &operator << (ostream &out, const LMatrix3d &mat);
INLINE ostream &operator << (ostream &out, const LMatrix4d &mat);
INLINE ostream &operator << (ostream &out, const LQuaterniond &q);
#endif
*/


#endif

