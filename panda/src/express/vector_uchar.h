// Filename: vector_uchar.h
// Created by:  drose (10May00)
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

#ifndef VECTOR_UCHAR_H
#define VECTOR_UCHAR_H

#include "pandabase.h"

#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : vector_uchar
// Description : A vector of uchars.  This class is defined once here,
//               and exported to PANDAEXPRESS.DLL; other packages that
//               want to use a vector of this type (whether they need
//               to export it or not) should include this header file,
//               rather than defining the vector again.
////////////////////////////////////////////////////////////////////

#define EXPCL EXPCL_PANDAEXPRESS
#define EXPTP EXPTP_PANDAEXPRESS
#define TYPE unsigned char
#define NAME vector_uchar

#include "vector_src.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
