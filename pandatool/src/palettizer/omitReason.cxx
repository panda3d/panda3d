// Filename: omitReason.cxx
// Created by:  drose (02Dec00)
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

#include "omitReason.h"

ostream &
operator << (ostream &out, OmitReason omit) {
  switch (omit) {
  case OR_none:
    return out << "none";

  case OR_working:
    return out << "working";

  case OR_omitted:
    return out << "omitted";

  case OR_size:
    return out << "size";

  case OR_solitary:
    return out << "solitary";

  case OR_coverage:
    return out << "coverage";

  case OR_unknown:
    return out << "unknown";

  case OR_unused:
    return out << "unused";
  }

  return out << "**invalid**(" << (int)omit << ")";
}
