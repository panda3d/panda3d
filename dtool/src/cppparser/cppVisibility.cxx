// Filename: cppVisibility.cxx
// Created by:  drose (22Oct99)
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
