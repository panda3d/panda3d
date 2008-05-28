// Filename: omitReason.cxx
// Created by:  drose (02Dec00)
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

  case OR_default_omit:
    return out << "default_omit";
  }

  return out << "**invalid**(" << (int)omit << ")";
}
