// Filename: lodNodeType.cxx
// Created by:  drose (08Jun07)
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

#include "lodNodeType.h"
#include "string_utils.h"

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
