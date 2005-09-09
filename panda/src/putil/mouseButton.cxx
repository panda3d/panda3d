// Filename: mouseButton.cxx
// Created by:  drose (01Mar00)
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

#include "mouseButton.h"
#include "buttonRegistry.h"
#include "notify.h"

#include <stdio.h>

ButtonHandle MouseButton::_buttons[num_mouse_buttons];
ButtonHandle MouseButton::_wheel_up;
ButtonHandle MouseButton::_wheel_down;

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
//     Function: MouseButton::four
//       Access: Public, Static
//  Description: Returns the ButtonHandle associated with the
//               fourth mouse button.
////////////////////////////////////////////////////////////////////
ButtonHandle MouseButton::
four() {
  return _buttons[3];
}

////////////////////////////////////////////////////////////////////
//     Function: MouseButton::five
//       Access: Public, Static
//  Description: Returns the ButtonHandle associated with the
//               fifth mouse button.
////////////////////////////////////////////////////////////////////
ButtonHandle MouseButton::
five() {
  return _buttons[4];
}

////////////////////////////////////////////////////////////////////
//     Function: MouseButton::wheel_up
//       Access: Public, Static
//  Description: Returns the ButtonHandle generated when the mouse
//               wheel is rolled one notch upwards.
////////////////////////////////////////////////////////////////////
ButtonHandle MouseButton::
wheel_up() {
  return _wheel_up;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseButton::wheel_down
//       Access: Public, Static
//  Description: Returns the ButtonHandle generated when the mouse
//               wheel is rolled one notch downwards.
////////////////////////////////////////////////////////////////////
ButtonHandle MouseButton::
wheel_down() {
  return _wheel_down;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseButton::is_mouse_button
//       Access: Public, Static
//  Description: Returns true if the indicated ButtonHandle is a mouse
//               button, false if it is some other kind of button.
////////////////////////////////////////////////////////////////////
bool MouseButton::
is_mouse_button(ButtonHandle button) {
  for (int i = 0; i < num_mouse_buttons; ++i) {
    if (button == _buttons[i]) {
      return true;
    }
  }

  return button == _wheel_up || button == _wheel_down;
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

  for (int i = 0; i < num_mouse_buttons; ++i) {
    sprintf(numstr, "mouse%d", i + 1);
    nassertv(strlen(numstr) < 20);

    ButtonRegistry::ptr()->register_button(_buttons[i], numstr);
  }

  ButtonRegistry::ptr()->register_button(_wheel_up, "wheel_up");
  ButtonRegistry::ptr()->register_button(_wheel_down, "wheel_down");
}
