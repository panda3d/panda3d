// Filename: parabola.h
// Created by:  drose (10Oct07)
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

#ifndef PARABOLA_H
#define PARABOLA_H

#include "pandabase.h"

#include "luse.h"
#include "indent.h"

#include "fltnames.h"
#include "parabola_src.h"

#include "dblnames.h"
#include "parabola_src.h"

#ifndef STDFLOAT_DOUBLE
typedef LParabolaf LParabola;
#else
typedef LParabolad LParabola;
#endif

#endif
