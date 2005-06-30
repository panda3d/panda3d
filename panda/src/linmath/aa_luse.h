// Filename: aa_luse.h
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
typedef LVecBase4f Colorf;
typedef LVecBase3f RGBColorf;

typedef LPoint3d Vertexd;
typedef LVector3d Normald;
typedef LPoint2d TexCoordd;
typedef LVecBase4d Colord;
typedef LVecBase3d RGBColord;


#endif

