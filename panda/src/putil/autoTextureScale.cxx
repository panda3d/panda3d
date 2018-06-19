/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file autoTextureScale.cxx
 * @author drose
 * @date 2011-11-28
 */

#include "autoTextureScale.h"
#include "string_utils.h"
#include "config_putil.h"

using std::istream;
using std::ostream;
using std::string;

ostream &
operator << (ostream &out, AutoTextureScale ats) {
  switch (ats) {
  case ATS_none:
    return out << "none";

  case ATS_down:
    return out << "down";

  case ATS_up:
    return out << "up";

  case ATS_pad:
    return out << "pad";

  case ATS_unspecified:
    return out << "unspecified";
  }

  return out << "**invalid AutoTextureScale (" << (int)ats << ")**";
}

istream &
operator >> (istream &in, AutoTextureScale &ats) {
  string word;
  in >> word;

  if (cmp_nocase(word, "none") == 0 ||
      cmp_nocase(word, "0") == 0 ||
      cmp_nocase(word, "#f") == 0 ||
      (!word.empty() && tolower(word[0]) == 'f')) {
    ats = ATS_none;

  } else if (cmp_nocase(word, "down") == 0 ||
             cmp_nocase(word, "1") == 0 ||
             cmp_nocase(word, "#t") == 0 ||
             (!word.empty() && tolower(word[0]) == 't')) {
    ats = ATS_down;

  } else if (cmp_nocase(word, "up") == 0) {
    ats = ATS_up;

  } else if (cmp_nocase(word, "pad") == 0) {
    ats = ATS_pad;

  } else {
    util_cat->error() << "Invalid AutoTextureScale value: " << word << "\n";
    ats = ATS_none;
  }

  return in;
}
