// Filename: distanceUnit.h
// Created by:  drose (17Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef DISTANCEUNIT_H
#define DISTANCEUNIT_H

#include <pandatoolbase.h>

////////////////////////////////////////////////////////////////////
// 	  Enum : DistanceUnit
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

ostream &operator << (ostream &out, DistanceUnit unit);
DistanceUnit string_distance_unit(const string &str);

double convert_units(DistanceUnit from, DistanceUnit to);

#endif
