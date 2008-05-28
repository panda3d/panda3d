// Filename: cppVisibility.cxx
// Created by:  drose (22Oct99)
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


#include "cppVisibility.h"

ostream &
operator << (ostream &out, CPPVisibility vis) {
  switch (vis) {
  case V_published:
    return out << "__published";

  case V_public:
    return out << "public";

  case V_protected:
    return out << "protected";

  case V_private:
    return out << "private";

  case V_unknown:
    return out << "unknown";
  }

  return out << "(**invalid visibility**)";
}
