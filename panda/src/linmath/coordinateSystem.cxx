// Filename: coordinateSystem.cxx
// Created by:  drose (24Sep99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "coordinateSystem.h"
#include "config_linmath.h"

#include "dconfig.h"
#include "notify.h"

#include <ctype.h>
#include <string>

CoordinateSystem default_coordinate_system;

CoordinateSystem
parse_coordinate_system_string(const string &str) {
  // First, make sure the string is lowercase before we compare it, so
  // we'll be case-insensitive.
  string lstr = str;
  for (string::iterator si = lstr.begin();
       si != lstr.end();
       ++si) {
    (*si) = tolower(*si);
  }

  if (lstr == "default") {
    return CS_default;

  } else if (lstr == "z-up" || lstr == "z-up-right") {
    return CS_zup_right;

  } else if (lstr == "y-up" || lstr == "y-up-right") {
    return CS_yup_right;

  } else if (lstr == "z-up-left") {
    return CS_zup_left;

  } else if (lstr == "y-up-left") {
    return CS_yup_left;
  }

  return CS_invalid;
}

INLINE_LINMATH bool
is_right_handed(CoordinateSystem cs) {
  if (cs == CS_default) {
    cs = default_coordinate_system;
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

  linmath_cat.error()
    << "Invalid coordinate_system value: " << (int)cs << "\n";
  nassertr(false, out);
  return out;
}
