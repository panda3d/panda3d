// Filename: buttonEvent.cxx
// Created by:  drose (01Mar00)
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

#include "buttonEvent.h"

////////////////////////////////////////////////////////////////////
//     Function: ButtonEvent::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void ButtonEvent::
output(ostream &out) const {
  switch (_type) {
  case T_down:
    out << "button " << _button << " down";
    break;

  case T_up:
    out << "button " << _button << " up";
    break;

  case T_keystroke:
    out << "keystroke " << _keycode;
    break;
  }
}
