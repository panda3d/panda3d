// Filename: parse_color.cxx
// Created by:  drose (25Aug11)
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

#include "parse_color.h"
#include <ctype.h>

static bool parse_hexdigit(int &result, char digit);

////////////////////////////////////////////////////////////////////
//     Function: parse_color
//  Description: Parses a HTML color spec of the form #rgb or #rrggbb.
//               Returns true on success, false on failure.  On
//               success, fills r, g, b with the color values in the
//               range 0..255.  On failure, r, g, b are undefined.
////////////////////////////////////////////////////////////////////
bool
parse_color(int &r, int &g, int &b, const string &color) {
  if (color.empty() || color[0] != '#') {
    return false;
  }
  if (color.length() == 4) {
    if (!parse_hexdigit(r, color[1]) || 
        !parse_hexdigit(g, color[2]) || 
        !parse_hexdigit(b, color[3])) {
      return false;
    }
    r *= 0x11;
    g *= 0x11;
    b *= 0x11;
    return true;
  }
  if (color.length() == 7) {
    int rh, rl, gh, gl, bh, bl;
    if (!parse_hexdigit(rh, color[1]) ||
        !parse_hexdigit(rl, color[2]) ||
        !parse_hexdigit(gh, color[3]) ||
        !parse_hexdigit(gl, color[4]) ||
        !parse_hexdigit(bh, color[5]) ||
        !parse_hexdigit(bl, color[6])) {
      return false;
    }
    r = (rh << 4) | rl;
    g = (gh << 4) | gl;
    b = (bh << 4) | bl;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: parse_hexdigit
//  Description: Parses a single hex digit.  Returns true on success,
//               false on failure.  On success, fills result with the
//               parsed value, an integer in the range 0..15.
////////////////////////////////////////////////////////////////////
bool
parse_hexdigit(int &result, char digit) {
  if (isdigit(digit)) {
    result = digit - '0';
    return true;
  } else if (isxdigit(digit)) {
    result = tolower(digit) - 'a' + 10;
    return true;
  }
  return false;
}
