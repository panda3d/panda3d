// Filename: mouseButton.cxx
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

#include "mouseButton.h"
#include "buttonRegistry.h"

#include <stdio.h>
#include "notify.h"

static const int num_mouse_buttons = 3;

static ButtonHandle _buttons[num_mouse_buttons];

////////////////////////////////////////////////////////////////////
//     Function: MouseButton::button
//       Access: Public, Static
//  Description: Returns the ButtonHandle associated with the
//               particular numbered mouse button (zero-based), if
//               there is one, or ButtonHandle::none() if there is
//               not.
////////////////////////////////////////////////////////////////////
ButtonHandle MouseButton::
button(int button_number) {
  if (button_number >= 0 && button_number < num_mouse_buttons) {
    return _buttons[button_number];
  }
  return ButtonHandle::none();
}

////////////////////////////////////////////////////////////////////
//     Function: MouseButton::one
//       Access: Public, Static
//  Description: Returns the ButtonHandle associated with the
//               first mouse button.
////////////////////////////////////////////////////////////////////
ButtonHandle MouseButton::
one() {
  return _buttons[0];
}

////////////////////////////////////////////////////////////////////
//     Function: MouseButton::two
//       Access: Public, Static
//  Description: Returns the ButtonHandle associated with the
//               second mouse button.
////////////////////////////////////////////////////////////////////
ButtonHandle MouseButton::
two() {
  return _buttons[1];
}

////////////////////////////////////////////////////////////////////
//     Function: MouseButton::three
//       Access: Public, Static
//  Description: Returns the ButtonHandle associated with the
//               third mouse button.
////////////////////////////////////////////////////////////////////
ButtonHandle MouseButton::
three() {
  return _buttons[2];
}

////////////////////////////////////////////////////////////////////
//     Function: MouseButton::is_mouse_button
//       Access: Public, Static
//  Description: Returns true if the indicated ButtonHandle is a mouse
//               button, false if it is some other kind of button.
////////////////////////////////////////////////////////////////////
bool MouseButton::
is_mouse_button(ButtonHandle button) {
  for (int i = 0; i < num_mouse_buttons; i++) {
    if (button == _buttons[i]) {
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseButton::init_mouse_buttons
//       Access: Public, Static
//  Description: This is intended to be called only once, by the
//               static initialization performed in config_util.cxx.
////////////////////////////////////////////////////////////////////
void MouseButton::
init_mouse_buttons() {
  char numstr[20];

  for (int i = 0; i < num_mouse_buttons; i++) {
    sprintf(numstr, "mouse%d", i + 1);
    nassertv(strlen(numstr) < 20);

    ButtonRegistry::ptr()->register_button(_buttons[i], numstr);
  }
}
