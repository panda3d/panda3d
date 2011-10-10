// Filename: vector_double.h
// Created by:  drose (10May00)
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

#ifndef VECTOR_DOUBLE_H
#define VECTOR_DOUBLE_H

#include "pandabase.h"

#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : vector_double
// Description : A vector of doubles.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a vector of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the vector again.
////////////////////////////////////////////////////////////////////

#define EXPCL EXPCL_PANDAEXPRESS
#define EXPTP EXPTP_PANDAEXPRESS
#define TYPE double
#define NAME vector_double

#include "vector_src.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
