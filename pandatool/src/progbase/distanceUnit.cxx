// Filename: distanceUnit.cxx
// Created by:  drose (17Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "distanceUnit.h"

#include <string_utils.h>

////////////////////////////////////////////////////////////////////
//     Function: DistanceUnit output operator
//  Description: 
////////////////////////////////////////////////////////////////////
ostream &
operator << (ostream &out, DistanceUnit unit) {
  switch (unit) {
  case DU_millimeters:
    return out << "mm";

  case DU_centimeters:
    return out << "cm";

  case DU_meters:
    return out << "m";

  case DU_kilometers:
    return out << "km";

  case DU_yards:
    return out << "yd";

  case DU_feet:
    return out << "ft";

  case DU_inches:
    return out << "in";

  case DU_nautical_miles:
    return out << "nmi";

  case DU_statute_miles:
    return out << "mi";

  case DU_invalid:
    return out << "invalid";
  }
  return out << "**unexpected DistanceUnit value: (" << (int)unit << ")**";
}

////////////////////////////////////////////////////////////////////
//     Function: string_distance_unit
//  Description: Converts from a string, as might be input by the
//               user, to one of the known DistanceUnit types.
//               Returns DU_invalid if the string is unknown.
////////////////////////////////////////////////////////////////////
DistanceUnit
string_distance_unit(const string &str) {
  if (cmp_nocase(str, "mm") == 0 || cmp_nocase(str, "millimeters") == 0) {
    return DU_millimeters;

  } else if (cmp_nocase(str, "cm") == 0 || cmp_nocase(str, "centimeters") == 0) {
    return DU_centimeters;

  } else if (cmp_nocase(str, "m") == 0 || cmp_nocase(str, "meters") == 0) {
    return DU_meters;

  } else if (cmp_nocase(str, "km") == 0 || cmp_nocase(str, "kilometers") == 0) {
    return DU_kilometers;
  
  } else if (cmp_nocase(str, "yd") == 0 || cmp_nocase(str, "yards") == 0) {
    return DU_yards;
  
  } else if (cmp_nocase(str, "ft") == 0 || cmp_nocase(str, "feet") == 0) {
    return DU_feet;
  
  } else if (cmp_nocase(str, "in") == 0 || cmp_nocase(str, "inches") == 0) {
    return DU_inches;
  
  } else if (cmp_nocase(str, "nmi") == 0 || 
	     cmp_nocase(str, "nm") == 0 || 
	     cmp_nocase_uh(str, "nautical_miles") == 0) {
    return DU_nautical_miles;
  
  } else if (cmp_nocase(str, "mi") == 0 || 
	     cmp_nocase(str, "miles") == 0 ||
	     cmp_nocase_uh(str, "statute_miles") == 0) {
    return DU_statute_miles;

  } else {
    return DU_invalid;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: unit_scale
//  Description: Returns the number of the indicated unit per each
//               centimeter.  This internal function is used to
//               implement convert_units(), below.
////////////////////////////////////////////////////////////////////
static double unit_scale(DistanceUnit unit) {
  switch (unit) {
  case DU_millimeters:
    return 0.1;

  case DU_centimeters:
    return 1.0;

  case DU_meters:
    return 100.0;

  case DU_kilometers:
    return 100000.0;

  case DU_yards:
    return 3.0 * 12.0 * 2.54;

  case DU_feet:
    return 12.0 * 2.54;

  case DU_inches:
    return 2.54;

  case DU_nautical_miles:
    // This is the U.S. definition.
    return 185200.0;

  case DU_statute_miles:
    return 5280.0 * 12.0 * 2.54;

  case DU_invalid:
    return 1.0;
  }

  return 1.0;
}

////////////////////////////////////////////////////////////////////
//     Function: convert_units
//  Description: Returns the scaling factor that must be applied to
//               convert from units of "from" to "to".
////////////////////////////////////////////////////////////////////
double convert_units(DistanceUnit from, DistanceUnit to) {
  return unit_scale(to) / unit_scale(from);
}
