// Filename: luse.h
// Created by:  drose (13Jan99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef LUSE_H
#define LUSE_H

////////////////////////////////////////////////////////////////////
//
// This file defines a number of vector-based classes that are
// designed for specific uses.  These all inherit from
// LVecBase[234][fd], which is the base of all linear algebra vectors.
//
// LPoint[234][fd]
//
//   This should be used to represent a specific point in space.  It
//   inherits most properties from LVecBase.
//
// LVector[234][fd]
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
// storing a plane equation or some such nonsense--use the base class,
// LVecBase.
//
// This file also typedefs the following:
//
// Vertex[fd]
// Normal[fd]
// TexCoord[fd]
// Color[fd]
// RGBColor[fd]
//
// These classes are typedefs of LPoint or LVector, as appropriate,
// and are intended to store a specific kind of rendering attribute.
// (Color is a four-component color; RGBColor is three-component.)
//
////////////////////////////////////////////////////////////////////

#include "pandabase.h"

#include "stl_compares.h"
#include "lvec2_ops.h"
#include "lvec3_ops.h"
#include "lvec4_ops.h"
#include "lmat_ops.h"
#include "lmatrix.h"
#include "lquaternion.h"
#include "lrotation.h"
#include "lorientation.h"
#include "lcast_to.h"

//ensure FLOATTYPE is set to float for macros are used outside of LINMATH
#include "fltnames.h"

// This macro defines the cast-to-another-numeric-type operator for
// all of the things defined in this package.  It works by virtue of
// there being an appropriate lcast_to() template function defined for
// each class.

#define LCAST(numeric_type, object) lcast_to((numeric_type *)0, object)


// Now we define some handy typedefs for these classes.
typedef LPoint3f Vertexf;
typedef LVector3f Normalf;
typedef LPoint2f TexCoordf;
typedef LVecBase4f Colorf;
typedef LVecBase3f RGBColorf;

typedef LPoint3d Vertexd;
typedef LVector3d Normald;
typedef LPoint2d TexCoordd;
typedef LVecBase4d Colord;
typedef LVecBase3d RGBColord;


#endif

