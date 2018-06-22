/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file colorSpace.cxx
 * @author rdb
 * @date 2014-06-02
 */

#include "colorSpace.h"
#include "config_putil.h"
#include "configVariableEnum.h"
#include "string_utils.h"

#include "dconfig.h"
#include "pnotify.h"

#include <ctype.h>

using std::istream;
using std::ostream;
using std::ostringstream;
using std::string;

ColorSpace
parse_color_space_string(const string &str) {
  if (cmp_nocase_uh(str, "linear") == 0 ||
             cmp_nocase_uh(str, "linear-rgb") == 0 ||
             cmp_nocase_uh(str, "lrgb") == 0) {
    return CS_linear;

  } else if (cmp_nocase_uh(str, "srgb") == 0) {
    return CS_sRGB;

  } else if (cmp_nocase_uh(str, "scrgb") == 0) {
    return CS_scRGB;

  } else if (cmp_nocase_uh(str, "unspecified") == 0) {
    return CS_unspecified;

  } else if (cmp_nocase_uh(str, "non-color") == 0) {
    // In case we want to add this as an enum value in the future.
    return CS_linear;
  }

  util_cat->error()
    << "Invalid color_space string: " << str << "\n";
  return CS_linear;
}

string
format_color_space(ColorSpace cs) {
  ostringstream strm;
  strm << cs;
  return strm.str();
}

ostream &
operator << (ostream &out, ColorSpace cs) {
  switch (cs) {
  case CS_linear:
    return out << "linear";

  case CS_sRGB:
    return out << "sRGB";

  case CS_scRGB:
    return out << "scRGB";

  case CS_unspecified:
    return out << "unspecified";
  }

  util_cat->error()
    << "Invalid color_space value: " << (int)cs << "\n";
  nassertr(false, out);
  return out;
}

istream &
operator >> (istream &in, ColorSpace &cs) {
  string word;
  in >> word;
  cs = parse_color_space_string(word);
  return in;
}
