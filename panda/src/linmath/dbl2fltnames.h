// Filename: dbl2fltnames.h
// Created by:  drose (04Apr01)
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


////////////////////////////////////////////////////////////////////
//
// This file is used particularly by lcast_to.h and lcast_to.cxx to
// define functions that convert from type double to type float.
//
////////////////////////////////////////////////////////////////////

#include "fltnames.h"

#undef FLOATTYPE2
#undef FLOATNAME2
#undef FLOATTOKEN2

#define FLOATTYPE2 double
#define FLOATNAME2(ARG) ARG##d
#define FLOATTOKEN2 'd'
