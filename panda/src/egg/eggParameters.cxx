/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggParameters.cxx
 * @author drose
 * @date 1999-01-16
 */

#include "eggParameters.h"

#include <string>

static EggParameters default_egg_parameters;
EggParameters *egg_parameters = &default_egg_parameters;


/**
 * Initializes all the parameters with default values.
 */
EggParameters::
EggParameters() {
  _pos_threshold = 0.0001;
  _normal_threshold = 0.0001;
  _uv_threshold = 0.0001;
  _color_threshold = 1.0/256.0;

  _table_threshold = 0.0001;
}


/**
 *
 */
EggParameters::
EggParameters(const EggParameters &other) {
  memcpy(this, &other, sizeof(EggParameters));
}
