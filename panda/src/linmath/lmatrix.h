// Filename: lmatrix.h
// Created by:  drose (15Jan99)
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

#ifndef LMATRIX_H
#define LMATRIX_H

#include "pandabase.h"
#include "config_linmath.h"

#include "lmatrix3.h"
#include "lmatrix4.h"

/*
typedef LMatrix3<float> LMatrix3f;
typedef LMatrix4<float> LMatrix4f;

typedef LMatrix3<double> LMatrix3d;
typedef LMatrix4<double> LMatrix4d;
*/

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
