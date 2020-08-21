/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lodNodeType.cxx
 * @author drose
 * @date 2007-06-08
 */

#include "lodNodeType.h"
#include "string_utils.h"
#include "config_pgraph.h"

using std::istream;
using std::ostream;
using std::string;

ostream &
operator << (ostream &out, LODNodeType lnt) {
  switch (lnt) {
  case LNT_pop:
    return out << "pop";

  case LNT_fade:
    return out << "fade";
  }

  pgraph_cat->error()
    << "Invalid LODNodeType value: " << (int)lnt << "\n";
  nassertr(false, out);
  return out;
}

istream &
operator >> (istream &in, LODNodeType &lnt) {
  string word;
  in >> word;
  if (cmp_nocase_uh(word, "pop") == 0) {
    lnt = LNT_pop;
  } else if (cmp_nocase_uh(word, "fade") == 0) {
    lnt = LNT_fade;
  } else {
    pgraph_cat->error()
      << "Invalid LODNodeType string: " << word << "\n";
    lnt = LNT_pop;
  }
  return in;
}
