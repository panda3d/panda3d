// Filename: deg_2_rad.h
// Created by:  drose (29Sep99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef DEG_2_RAD_H
#define DEG_2_RAD_H

#include <pandabase.h>

#include "mathNumbers.h"

BEGIN_PUBLISH
INLINE_LINMATH double deg_2_rad( double f ) { return f * (MathNumbers::pi / 180.0); }
INLINE_LINMATH double rad_2_deg( double f ) { return f * (180.0 / MathNumbers::pi); }

INLINE_LINMATH float deg_2_rad( float f ) { return f * (MathNumbers::pi_f / 180.0f); }
INLINE_LINMATH float rad_2_deg( float f ) { return f * (180.0f / MathNumbers::pi_f); }
END_PUBLISH

#endif

