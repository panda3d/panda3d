// Filename: distanceUnit.h
// Created by:  drose (17Apr01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
DistanceUnit string_distance_unit(const string &str);

double convert_units(DistanceUnit from, DistanceUnit to);

#endif
