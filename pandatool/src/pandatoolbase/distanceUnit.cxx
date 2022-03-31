/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file distanceUnit.cxx
 * @author drose
 * @date 2001-04-17
 */

#include "distanceUnit.h"
#include "config_pandatoolbase.h"
#include "string_utils.h"
#include "pnotify.h"

using std::istream;
using std::ostream;
using std::string;

/**
 * Returns the string representing the common abbreviation for the given unit.
 */
string
format_abbrev_unit(DistanceUnit unit) {
  switch (unit) {
  case DU_millimeters:
    return "mm";

  case DU_centimeters:
    return "cm";

  case DU_meters:
    return "m";

  case DU_kilometers:
    return "km";

  case DU_yards:
    return "yd";

  case DU_feet:
    return "ft";

  case DU_inches:
    return "in";

  case DU_nautical_miles:
    return "nmi";

  case DU_statute_miles:
    return "mi";

  case DU_invalid:
    return "invalid";
  }
  nout << "**unexpected DistanceUnit value: (" << (int)unit << ")**";
  return "**";
}

/**
 * Returns the string representing the full name (plural) for the given unit.
 */
string
format_long_unit(DistanceUnit unit) {
  switch (unit) {
  case DU_millimeters:
    return "millimeters";

  case DU_centimeters:
    return "centimeters";

  case DU_meters:
    return "meters";

  case DU_kilometers:
    return "kilometers";

  case DU_yards:
    return "yards";

  case DU_feet:
    return "feet";

  case DU_inches:
    return "inches";

  case DU_nautical_miles:
    return "nautical miles";

  case DU_statute_miles:
    return "miles";

  case DU_invalid:
    return "invalid";
  }
  nout << "**unexpected DistanceUnit value: (" << (int)unit << ")**";
  return "**";
}

/**
 *
 */
ostream &
operator << (ostream &out, DistanceUnit unit) {
  return out << format_abbrev_unit(unit);
}

/**
 *
 */
istream &
operator >> (istream &in, DistanceUnit &unit) {
  string word;
  in >> word;
  unit = string_distance_unit(word);
  if (unit == DU_invalid) {
    pandatoolbase_cat->error()
      << "Invalid distance unit: " << word << "\n";
  }
  return in;
}

/**
 * Converts from a string, as might be input by the user, to one of the known
 * DistanceUnit types.  Returns DU_invalid if the string is unknown.
 */
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

/**
 * Returns the number of the indicated unit per each centimeter.  This
 * internal function is used to implement convert_units(), below.
 */
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

/**
 * Returns the scaling factor that must be applied to convert from units of
 * "from" to "to".
 */
double convert_units(DistanceUnit from, DistanceUnit to) {
  return unit_scale(from) / unit_scale(to);
}
