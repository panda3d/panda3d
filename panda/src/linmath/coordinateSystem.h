/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file coordinateSystem.h
 * @author drose
 * @date 1999-09-24
 */

#ifndef COORDINATESYSTEM_H
#define COORDINATESYSTEM_H

#include "pandabase.h"

#include "typedef.h"

BEGIN_PUBLISH

enum CoordinateSystem {
  // The CS_default entry does not refer to a particular coordinate system,
  // but rather to the value stored in default_coordinate_system, which in
  // turn is loaded from the config variable "coordinate-system".
  CS_default,

  CS_zup_right, // Z-Up, Right-handed
  CS_yup_right, // Y-Up, Right-handed
  CS_zup_left,  // Z-Up, Left-handed
  CS_yup_left,  // Y-Up, Left-handed

  // CS_invalid is not a coordinate system at all.  It can be used in user-
  // input processing code to indicate a contradictory coordinate system
  // request.
  CS_invalid,
};

EXPCL_PANDA_LINMATH CoordinateSystem get_default_coordinate_system();
EXPCL_PANDA_LINMATH CoordinateSystem parse_coordinate_system_string(const std::string &str);
EXPCL_PANDA_LINMATH std::string format_coordinate_system(CoordinateSystem cs);
EXPCL_PANDA_LINMATH bool is_right_handed(CoordinateSystem cs = CS_default);

END_PUBLISH

#define IS_LEFT_HANDED_COORDSYSTEM(cs) ((cs==CS_zup_left) || (cs==CS_yup_left))

EXPCL_PANDA_LINMATH std::ostream &operator << (std::ostream &out, CoordinateSystem cs);
EXPCL_PANDA_LINMATH std::istream &operator >> (std::istream &in, CoordinateSystem &cs);


#endif
