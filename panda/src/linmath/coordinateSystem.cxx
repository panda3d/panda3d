/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file coordinateSystem.cxx
 * @author drose
 * @date 1999-09-24
 */

#include "coordinateSystem.h"
#include "config_linmath.h"
#include "configVariableEnum.h"
#include "string_utils.h"

#include "dconfig.h"
#include "pnotify.h"

#include <ctype.h>

using std::istream;
using std::ostream;
using std::ostringstream;
using std::string;

static ConfigVariableEnum<CoordinateSystem> default_cs
("coordinate-system", CS_zup_right,
 PRC_DESC("The default coordinate system to use throughout Panda for "
          "rendering, user input, and matrix operations, unless specified "
          "otherwise."));


CoordinateSystem
get_default_coordinate_system() {
  CoordinateSystem cs = default_cs;
  return (cs == CS_default || cs == CS_invalid) ? CS_zup_right : cs;
}


CoordinateSystem
parse_coordinate_system_string(const string &str) {
  if (cmp_nocase_uh(str, "default") == 0) {
    return CS_default;

  } else if (cmp_nocase_uh(str, "zup") == 0 ||
             cmp_nocase_uh(str, "zup-right") == 0 ||
             cmp_nocase_uh(str, "z-up") == 0 ||
             cmp_nocase_uh(str, "z-up-right") == 0) {
    return CS_zup_right;

  } else if (cmp_nocase_uh(str, "yup") == 0 ||
             cmp_nocase_uh(str, "yup-right") == 0 ||
             cmp_nocase_uh(str, "y-up") == 0 ||
             cmp_nocase_uh(str, "y-up-right") == 0) {
    return CS_yup_right;

  } else if (cmp_nocase_uh(str, "z-up-left") == 0 ||
             cmp_nocase_uh(str, "zup-left") == 0) {
    return CS_zup_left;

  } else if (cmp_nocase_uh(str, "y-up-left") == 0 ||
             cmp_nocase_uh(str, "yup-left") == 0) {
    return CS_yup_left;
  }

  return CS_invalid;
}

string
format_coordinate_system(CoordinateSystem cs) {
  ostringstream strm;
  strm << cs;
  return strm.str();
}

bool
is_right_handed(CoordinateSystem cs) {
  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }
  switch (cs) {
  case CS_zup_right:
  case CS_yup_right:
    return true;

  case CS_zup_left:
  case CS_yup_left:
    return false;

  default:
    linmath_cat.error()
      << "Invalid coordinate system value: " << (int)cs << "\n";
    nassertr(false, false);
    return false;
  }
}

ostream &
operator << (ostream &out, CoordinateSystem cs) {
  switch (cs) {
  case CS_default:
    return out << "default";

  case CS_zup_right:
    return out << "zup_right";

  case CS_yup_right:
    return out << "yup_right";

  case CS_zup_left:
    return out << "zup_left";

  case CS_yup_left:
    return out << "yup_left";

  case CS_invalid:
    return out << "invalid";
  }

  linmath_cat->error()
    << "Invalid coordinate_system value: " << (int)cs << "\n";
  nassertr(false, out);
  return out;
}

istream &
operator >> (istream &in, CoordinateSystem &cs) {
  string word;
  in >> word;
  cs = parse_coordinate_system_string(word);
  if (cs == CS_invalid) {
    linmath_cat->error()
      << "Invalid coordinate_system string: " << word << "\n";
  }
  return in;
}
