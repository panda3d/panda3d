/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file deg_2_rad.h
 * @author drose
 * @date 1999-09-29
 */

#ifndef DEG_2_RAD_H
#define DEG_2_RAD_H

#include "pandabase.h"

#include "mathNumbers.h"

BEGIN_PUBLISH
INLINE_LINMATH double deg_2_rad(double f);
INLINE_LINMATH double rad_2_deg(double f);

INLINE_LINMATH float deg_2_rad(float f);
INLINE_LINMATH float rad_2_deg(float f);
END_PUBLISH

#include "deg_2_rad.I"

#endif
