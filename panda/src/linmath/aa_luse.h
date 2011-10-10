// Filename: aa_luse.h
// Created by:  drose (13Jan99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef AA_LUSE_H
#define AA_LUSE_H

// This file is include by luse.h to do all the work required by that
// header file.  It is in a separate header file to avoid cyclic
// header dependencies, and because interrogate wants to sort header
// files in alphabetical order and this one should pretty much be
// included first.

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
typedef LPoint3f TexCoord3f;
typedef LVecBase4f Colorf;
typedef LVecBase3f RGBColorf;

typedef LPoint3d Vertexd;
typedef LVector3d Normald;
typedef LPoint2d TexCoordd;
typedef LPoint3d TexCoord3d;
typedef LVecBase4d Colord;
typedef LVecBase3d RGBColord;

// And finally, we define the unqualified "standard" float type, which
// is based on the setting of STDFLOAT_DOUBLE.  This is the type that
// is used for graphics-specific operations such as vertex and pos
// value.  The default is single-precision floats, which is almost
// always what you really want.
#ifndef STDFLOAT_DOUBLE
// The default setting--single-precision floats.

typedef LVecBase2f LVecBase2;
typedef LPoint2f LPoint2;
typedef LVector2f LVector2;
typedef LVecBase3f LVecBase3;
typedef LPoint3f LPoint3;
typedef LVector3f LVector3;
typedef LVecBase4f LVecBase4;
typedef LPoint4f LPoint4;
typedef LVector4f LVector4;
typedef LQuaternionf LQuaternion;
typedef LRotationf LRotation;
typedef LOrientationf LOrientation;
typedef LMatrix3f LMatrix3;
typedef LMatrix4f LMatrix4;

typedef Vertexf LVertex;
typedef Normalf LNormal;
typedef TexCoordf LTexCoord;
typedef TexCoord3f LTexCoord3;
typedef Colorf LColor;
typedef RGBColorf LRGBColor;

#else  // STDFLOAT_DOUBLE
// The specialty setting--double-precision floats.

typedef LVecBase2d LVecBase2;
typedef LPoint2d LPoint2;
typedef LVector2d LVector2;
typedef LVecBase3d LVecBase3;
typedef LPoint3d LPoint3;
typedef LVector3d LVector3;
typedef LVecBase4d LVecBase4;
typedef LPoint4d LPoint4;
typedef LVector4d LVector4;
typedef LQuaterniond LQuaternion;
typedef LRotationd LRotation;
typedef LOrientationd LOrientation;
typedef LMatrix3d LMatrix3;
typedef LMatrix4d LMatrix4;

typedef Vertexd LVertex;
typedef Normald LNormal;
typedef TexCoordd LTexCoord;
typedef TexCoord3d LTexCoord3;
typedef Colord LColor;
typedef RGBColord LRGBColor;

#endif  // STDFLOAT_DOUBLE

#endif
