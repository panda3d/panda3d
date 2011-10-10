// Filename: vector_stdfloat.h
// Created by:  drose (07Oct11)
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

#ifndef VECTOR_STDFLOAT_H
#define VECTOR_STDFLOAT_H

#include "pandabase.h"

#include "vector_double.h"
#include "vector_float.h"

#ifndef STDFLOAT_DOUBLE
typedef vector_float vector_stdfloat;
#else
typedef vector_double vector_stdfloat;
#endif  // STDFLOAT_DOUBLE

#endif

