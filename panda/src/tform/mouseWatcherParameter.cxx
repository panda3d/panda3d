// Filename: mouseWatcherParameter.cxx
// Created by:  drose (06Jul01)
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

#include "mouseWatcherParameter.h"

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherParameter::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void MouseWatcherParameter::
output(ostream &out) const {
  bool output_anything = false;

  if (has_button()) {
    out << _button;
    output_anything = true;
  }

  if (has_keycode()) {
    if (output_anything) {
      out << ", ";
    }
    out << "key" << _keycode;
    output_anything = true;
  }

  if (_mods.is_any_down()) {
    if (output_anything) {
      out << ", ";
    }
    out << _mods;
    output_anything = true;
  }

  if (has_mouse()) {
    if (output_anything) {
      out << ", ";
    }
    out << "(" << _mouse << ")";
    output_anything = true;
  }

  if (is_outside()) {
    if (output_anything) {
      out << ", ";
    }
    out << "outside";
    output_anything = true;
  }

  if (!output_anything) {
    out << "no parameters";
  }
}
