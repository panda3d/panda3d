// Filename: pointShapeProperty.cxx
// Created by:  charles (10Jul00)
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

#include "pointShapeProperty.h"

ostream &
operator << (ostream &out, PointShapeProperty::Mode mode) {
  switch (mode) {
  case PointShapeProperty::M_square:
    return out << "square";

  case PointShapeProperty::M_circle:
    return out << "circle";
  }

  return out << "**invalid**(" << (int)mode << ")";
}

////////////////////////////////////////////////////////////////////
// Function : output
//   Access : public
////////////////////////////////////////////////////////////////////
void PointShapeProperty::
output(ostream &out) const {
  out << _mode;
}
