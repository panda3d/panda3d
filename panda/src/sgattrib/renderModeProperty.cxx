// Filename: renderModeProperty.cxx
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

#include "renderModeProperty.h"

ostream &
operator << (ostream &out, RenderModeProperty::Mode mode) {
  switch (mode) {
  case RenderModeProperty::M_filled:
    return out << "filled";

  case RenderModeProperty::M_wireframe:
    return out << "wireframe";
  }

  return out << "**invalid**(" << (int)mode << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeProperty::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void RenderModeProperty::
output(ostream &out) const {
  out << _mode << ":" << _line_width;
}
