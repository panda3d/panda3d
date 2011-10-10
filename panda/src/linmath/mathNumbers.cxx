// Filename: mathNumbers.cxx
// Created by:  mike (24Sep99)
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

#include "mathNumbers.h"
#include <math.h>

const double MathNumbers::pi_d = 4.0 * atan(1.0);
const double MathNumbers::ln2_d = log(2.0);
const double MathNumbers::rad_2_deg_d = 180.0 / MathNumbers::pi_d;
const double MathNumbers::deg_2_rad_d = MathNumbers::pi_d / 180.0;

const float MathNumbers::pi_f = 4.0 * atan(1.0);
const float MathNumbers::ln2_f = log(2.0);
const float MathNumbers::rad_2_deg_f = 180.0 / MathNumbers::pi_d;
const float MathNumbers::deg_2_rad_f = MathNumbers::pi_d / 180.0;

const PN_stdfloat MathNumbers::pi = 4.0 * atan(1.0);
const PN_stdfloat MathNumbers::ln2 = log(2.0);
const PN_stdfloat MathNumbers::rad_2_deg = 180.0 / MathNumbers::pi_d;
const PN_stdfloat MathNumbers::deg_2_rad = MathNumbers::pi_d / 180.0;

