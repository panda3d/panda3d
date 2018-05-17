/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mouseButton.cxx
 * @author drose
 * @date 2000-03-01
 */

#include "mouseButton.h"
#include "buttonRegistry.h"
#include "pnotify.h"

#include <stdio.h>

ButtonHandle MouseButton::_buttons[num_mouse_buttons];
ButtonHandle MouseButton::_wheel_up;
ButtonHandle MouseButton::_wheel_down;
ButtonHandle MouseButton::_wheel_left;
ButtonHandle MouseButton::_wheel_right;

/**
 * Returns the ButtonHandle associated with the particular numbered mouse
 * button (zero-based), if there is one, or ButtonHandle::none() if there is
 * not.
 */
ButtonHandle MouseButton::
button(int button_number) {
  if (button_number >= 0 && button_number < num_mouse_buttons) {
    return _buttons[button_number];
  }
  return ButtonHandle::none();
}

/**
 * Returns the ButtonHandle associated with the first mouse button.
 */
ButtonHandle MouseButton::
one() {
  return _buttons[0];
}

/**
 * Returns the ButtonHandle associated with the second mouse button.
 */
ButtonHandle MouseButton::
two() {
  return _buttons[1];
}

/**
 * Returns the ButtonHandle associated with the third mouse button.
 */
ButtonHandle MouseButton::
three() {
  return _buttons[2];
}

/**
 * Returns the ButtonHandle associated with the fourth mouse button.
 */
ButtonHandle MouseButton::
four() {
  return _buttons[3];
}

/**
 * Returns the ButtonHandle associated with the fifth mouse button.
 */
ButtonHandle MouseButton::
five() {
  return _buttons[4];
}

/**
 * Returns the ButtonHandle generated when the mouse wheel is rolled one notch
 * upwards.
 */
ButtonHandle MouseButton::
wheel_up() {
  return _wheel_up;
}

/**
 * Returns the ButtonHandle generated when the mouse wheel is rolled one notch
 * downwards.
 */
ButtonHandle MouseButton::
wheel_down() {
  return _wheel_down;
}

/**
 * Returns the ButtonHandle generated when the mouse is scrolled to the left.
 * Usually, you'll only find the horizontal scroll on laptops.
 */
ButtonHandle MouseButton::
wheel_left() {
  return _wheel_left;
}

/**
 * Returns the ButtonHandle generated when the mouse is scrolled to the right.
 * Usually, you'll only find the horizontal scroll on laptops.
 */
ButtonHandle MouseButton::
wheel_right() {
  return _wheel_right;
}

/**
 * Returns true if the indicated ButtonHandle is a mouse button, false if it
 * is some other kind of button.
 */
bool MouseButton::
is_mouse_button(ButtonHandle button) {
  for (int i = 0; i < num_mouse_buttons; ++i) {
    if (button == _buttons[i]) {
      return true;
    }
  }

  return button == _wheel_up || button == _wheel_down || button == _wheel_left || button == _wheel_right;
}

/**
 * This is intended to be called only once, by the static initialization
 * performed in config_putil.cxx.
 */
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
  ButtonRegistry::ptr()->register_button(_wheel_left, "wheel_left");
  ButtonRegistry::ptr()->register_button(_wheel_right, "wheel_right");
}
