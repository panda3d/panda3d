// Filename: deg_2_rad.h
// Created by:  drose (29Sep99)
//
////////////////////////////////////////////////////////////////////

#ifndef DEG_2_RAD_H
#define DEG_2_RAD_H

#include <pandabase.h>

#include "mathNumbers.h"

BEGIN_PUBLISH
INLINE_LINMATH double deg_2_rad( double f ) { return f * MathNumbers::pi / 180.0; }
INLINE_LINMATH double rad_2_deg( double f ) { return f * 180.0 / MathNumbers::pi; }
END_PUBLISH

#endif

