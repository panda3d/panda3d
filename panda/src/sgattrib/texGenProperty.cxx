// Filename: texGenProperty.cxx
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

#include "texGenProperty.h"

ostream &
operator << (ostream &out, TexGenProperty::Mode mode) {
  switch (mode) {
  case TexGenProperty::M_none:
    return out << "none";

  case TexGenProperty::M_eye_linear:
    return out << "eye_linear";

  case TexGenProperty::M_texture_projector:
    return out << "texture_projector";

  case TexGenProperty::M_sphere_map:
    return out << "sphere_map";
  }

  return out << "**invalid**(" << (int)mode << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: TexGenProperty::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void TexGenProperty::
output(ostream &out) const {
  out << _mode;
}
