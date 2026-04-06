/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file aa_luse.h
 * @author drose
 * @date 1999-01-13
 */

#ifndef AA_LUSE_H
#define AA_LUSE_H

// This file is include by luse.h to do all the work required by that header
// file.  It is in a separate header file to avoid cyclic header dependencies,
// and because interrogate wants to sort header files in alphabetical order
// and this one should pretty much be included first.

#include "pandabase.h"

#include "lsimpleMatrix.h"
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

// ensure FLOATTYPE is set to float for macros are used outside of LINMATH
#include "fltnames.h"

// This macro defines the cast-to-another-numeric-type operator for all of the
// things defined in this package.  It works by virtue of there being an
// appropriate lcast_to() template function defined for each class.

#define LCAST(numeric_type, object) lcast_to((numeric_type *)0, object)

BEGIN_PUBLISH

// Now we define some handy typedefs for these classes.
typedef LPoint3f LVertexf;
typedef LVector3f LNormalf;
typedef LPoint2f LTexCoordf;
typedef LPoint3f LTexCoord3f;
typedef LVecBase4f LColorf;
typedef LVecBase3f LRGBColorf;

typedef LPoint3d LVertexd;
typedef LVector3d LNormald;
typedef LPoint2d LTexCoordd;
typedef LPoint3d LTexCoord3d;
typedef LVecBase4d LColord;
typedef LVecBase3d LRGBColord;

// These used to be separate classes, but now they're just typedefs.
typedef LVecBase4f UnalignedLVecBase4f;
typedef LVecBase4d UnalignedLVecBase4d;
typedef LVecBase4i UnalignedLVecBase4i;
typedef LMatrix4f UnalignedLMatrix4f;
typedef LMatrix4d UnalignedLMatrix4d;

// The following names are only for legacy Python code.  These aren't real
// typedefs; they're just commands to interrogate.
#ifdef CPPPARSER
typedef LMatrix4f Mat4F;
typedef LMatrix3f Mat3F;
typedef LVecBase4f VBase4F;
typedef LVector4f Vec4F;
typedef LPoint4f Point4F;
typedef LVecBase3f VBase3F;
typedef LVector3f Vec3F;
typedef LPoint3f Point3F;
typedef LVecBase2f VBase2F;
typedef LVector2f Vec2F;
typedef LPoint2f Point2F;
typedef LQuaternionf QuatF;
typedef LMatrix4d Mat4D;
typedef LMatrix3d Mat3D;
typedef LVecBase4d VBase4D;
typedef LVector4d Vec4D;
typedef LPoint4d Point4D;
typedef LVecBase3d VBase3D;
typedef LVector3d Vec3D;
typedef LPoint3d Point3D;
typedef LVecBase2d VBase2D;
typedef LVector2d Vec2D;
typedef LPoint2d Point2D;
typedef LQuaterniond QuatD;
#endif // CPPPARSER

// And finally, we define the unqualified "standard" float type, which is
// based on the setting of STDFLOAT_DOUBLE.  This is the type that is used for
// graphics-specific operations such as vertex and pos value.  The default is
// single-precision floats, which is almost always what you really want.
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

typedef LVertexf LVertex;
typedef LNormalf LNormal;
typedef LTexCoordf LTexCoord;
typedef LTexCoord3f LTexCoord3;
typedef LColorf LColor;
typedef LRGBColorf LRGBColor;

typedef UnalignedLVecBase4f UnalignedLVecBase4;
typedef UnalignedLMatrix4f UnalignedLMatrix4;

// Bogus typedefs for interrogate and legacy Python code.
#ifdef CPPPARSER
typedef LMatrix4f Mat4;
typedef LMatrix3f Mat3;
typedef LVecBase4f VBase4;
typedef LVector4f Vec4;
typedef LPoint4f Point4;
typedef LVecBase3f VBase3;
typedef LVector3f Vec3;
typedef LPoint3f Point3;
typedef LVecBase2f VBase2;
typedef LVector2f Vec2;
typedef LPoint2f Point2;
typedef LQuaternionf Quat;
#endif // CPPPARSER

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

typedef LVertexd LVertex;
typedef LNormald LNormal;
typedef LTexCoordd LTexCoord;
typedef LTexCoord3d LTexCoord3;
typedef LColord LColor;
typedef LRGBColord LRGBColor;

typedef UnalignedLVecBase4d UnalignedLVecBase4;
typedef UnalignedLMatrix4d UnalignedLMatrix4;

// Bogus typedefs for interrogate and legacy Python code.
#ifdef CPPPARSER
typedef LMatrix4d Mat4;
typedef LMatrix3d Mat3;
typedef LVecBase4d VBase4;
typedef LVector4d Vec4;
typedef LPoint4d Point4;
typedef LVecBase3d VBase3;
typedef LVector3d Vec3;
typedef LPoint3d Point3;
typedef LVecBase2d VBase2;
typedef LVector2d Vec2;
typedef LPoint2d Point2;
typedef LQuaterniond Quat;
#endif // CPPPARSER

#endif  // STDFLOAT_DOUBLE

END_PUBLISH

#endif
