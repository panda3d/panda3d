// Filename: mathNumbers.cxx
// Created by:  mike (24Sep99)
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

#include "mathNumbers.h"
#include <math.h>

const double MathNumbers::pi = 4.0 * atan(1.0);
const double MathNumbers::ln2 = log(2.0);
const double MathNumbers::rad_2_deg = 180.0 / MathNumbers::pi;
const double MathNumbers::deg_2_rad = MathNumbers::pi / 180.0;

const float MathNumbers::pi_f = 4.0 * atan(1.0);
const float MathNumbers::ln2_f = log(2.0);
const float MathNumbers::rad_2_deg_f = 180.0 / MathNumbers::pi;
const float MathNumbers::deg_2_rad_f = MathNumbers::pi / 180.0;

