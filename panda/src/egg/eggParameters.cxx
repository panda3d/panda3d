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
