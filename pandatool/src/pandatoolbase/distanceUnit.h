/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file distanceUnit.h
 * @author drose
 * @date 2001-04-17
 */

#ifndef DISTANCEUNIT_H
#define DISTANCEUNIT_H

#include "pandatoolbase.h"

/**
 * This enumerated type lists all the kinds of units we're likely to come
 * across in model conversion programs.
 */
enum DistanceUnit {
  DU_millimeters,
  DU_centimeters,
  DU_meters,
  DU_kilometers,
  DU_yards,
  DU_feet,
  DU_inches,
  DU_nautical_miles,
  DU_statute_miles,
  DU_invalid
};

std::string format_abbrev_unit(DistanceUnit unit);
std::string format_long_unit(DistanceUnit unit);

std::ostream &operator << (std::ostream &out, DistanceUnit unit);
std::istream &operator >> (std::istream &in, DistanceUnit &unit);
DistanceUnit string_distance_unit(const std::string &str);

double convert_units(DistanceUnit from, DistanceUnit to);

#endif
