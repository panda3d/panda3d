// Filename: vector_LVecBase3f.h
// Created by:  drose (11Dec00)
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

#ifndef VECTOR_LVECBASE3F_H
#define VECTOR_LVECBASE3F_H

#include "pandabase.h"
#include "luse.h"
#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : vector_LVecBase3f
// Description : A vector of LVecBase3fs.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a vector of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the vector again.
////////////////////////////////////////////////////////////////////

#define EXPCL EXPCL_PANDA
#define EXPTP EXPTP_PANDA
#define TYPE LVecBase3f
#define NAME vector_LVecBase3f

#include "vector_src.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
