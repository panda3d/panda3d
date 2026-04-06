/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dbl2fltnames.h
 * @author drose
 * @date 2001-04-04
 */

// This file is used particularly by lcast_to.h and lcast_to.cxx to define
// functions that convert from type double to type float.

#include "fltnames.h"

#undef FLOATTYPE2
#undef FLOATNAME2
#undef FLOATTOKEN2

#define FLOATTYPE2 double
#define FLOATNAME2(ARG) ARG##d
#define FLOATTOKEN2 'd'
