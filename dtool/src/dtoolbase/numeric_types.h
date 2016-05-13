/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file numeric_types.h
 * @author drose
 * @date 2000-06-06
 */

#ifndef NUMERIC_TYPES_H
#define NUMERIC_TYPES_H

#include "dtoolbase.h"

// This header file defines a number of typedefs that correspond to the
// various numeric types for unsigned and signed numbers of various widths.

#include <stdint.h>

typedef double PN_float64;
typedef float PN_float32;

#ifndef STDFLOAT_DOUBLE
// The default setting--single-precision floats.
typedef float PN_stdfloat;
#else  // STDFLOAT_DOUBLE
// The specialty setting--double-precision floats.
typedef double PN_stdfloat;
#endif  // STDFLOAT_DOUBLE

#endif
