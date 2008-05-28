// Filename: distanceUnit.h
// Created by:  drose (17Apr01)
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

#ifndef DISTANCEUNIT_H
#define DISTANCEUNIT_H

#include "pandatoolbase.h"

////////////////////////////////////////////////////////////////////
//        Enum : DistanceUnit
// Description : This enumerated type lists all the kinds of units
//               we're likely to come across in model conversion
//               programs.
////////////////////////////////////////////////////////////////////
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

string format_abbrev_unit(DistanceUnit unit);
string format_long_unit(DistanceUnit unit);

ostream &operator << (ostream &out, DistanceUnit unit);
istream &operator >> (istream &in, DistanceUnit &unit);
DistanceUnit string_distance_unit(const string &str);

double convert_units(DistanceUnit from, DistanceUnit to);

#endif
