// Filename: colorBlendProperty.cxx
// Created by:  drose (22Mar00)
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

#include "colorBlendProperty.h"

ostream &
operator << (ostream &out, ColorBlendProperty::Mode mode) {
  switch (mode) {
  case ColorBlendProperty::M_none:
    return out << "none";

  case ColorBlendProperty::M_multiply:
    return out << "multiply";

  case ColorBlendProperty::M_add:
    return out << "add";

  case ColorBlendProperty::M_multiply_add:
    return out << "multiply_add";

  case ColorBlendProperty::M_alpha:
    return out << "alpha";
  }

  return out << "**invalid**(" << (int)mode << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendProperty::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void ColorBlendProperty::
output(ostream &out) const {
  out << _mode;
}
