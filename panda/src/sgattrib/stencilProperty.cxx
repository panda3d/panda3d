// Filename: stencilProperty.cxx
// Created by:  drose (23Mar00)
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

#include "stencilProperty.h"

ostream &
operator << (ostream &out, StencilProperty::Mode mode) {
  switch (mode) {
  case StencilProperty::M_none:
    return out << "none";

  case StencilProperty::M_never:
    return out << "never";

  case StencilProperty::M_less:
    return out << "less";

  case StencilProperty::M_equal:
    return out << "equal";

  case StencilProperty::M_less_equal:
    return out << "less_equal";

  case StencilProperty::M_greater:
    return out << "greater";

  case StencilProperty::M_not_equal:
    return out << "not_equal";

  case StencilProperty::M_greater_equal:
    return out << "greater_equal";

  case StencilProperty::M_always:
    return out << "always";
  }

  return out << "**invalid**(" << (int)mode << ")";
}

ostream &
operator << (ostream &out, StencilProperty::Action action) {
  switch (action) {
  case StencilProperty::A_keep:
    return out << "keep";

  case StencilProperty::A_zero:
    return out << "zero";

  case StencilProperty::A_replace:
    return out << "replace";

  case StencilProperty::A_increment:
    return out << "increment";

  case StencilProperty::A_decrement:
    return out << "decrement";

  case StencilProperty::A_invert:
    return out << "invert";
  }

  return out << "**invalid**(" << (int)action << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: StencilProperty::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void StencilProperty::
output(ostream &out) const {
  out << _mode;
  if (_mode != M_none) {
    out << ":" << _action;
  }
}
