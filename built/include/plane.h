/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file plane.h
 * @author mike
 * @date 1997-01-09
 */

#ifndef PLANE_H
#define PLANE_H

#include "pandabase.h"

#include "luse.h"
#include "indent.h"
#include "nearly_zero.h"
#include "cmath.h"
#include "parabola.h"

class Datagram;
class DatagramIterator;

#include "fltnames.h"
#include "plane_src.h"

#include "dblnames.h"
#include "plane_src.h"

#ifndef STDFLOAT_DOUBLE
typedef LPlanef LPlane;
#else
typedef LPlaned LPlane;
#endif

// Bogus typedefs for interrogate and legacy Python code.
#ifdef CPPPARSER
typedef LPlanef PlaneF;
typedef LPlaned PlaneD;
#ifndef STDFLOAT_DOUBLE
typedef LPlanef Plane;
#else
typedef LPlaned Plane;
#endif
#endif  // CPPPARSER

#endif
