/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file gamepadButton.cxx
 * @author rdb
 * @date 2015-08-21
 */

#include "gamepadButton.h"
#include "buttonRegistry.h"

#define DEFINE_GAMEPAD_BUTTON_HANDLE(KeyName)     \
                  static ButtonHandle _##KeyName; \
                  ButtonHandle GamepadButton::KeyName() { return _##KeyName; }

DEFINE_GAMEPAD_BUTTON_HANDLE(lstick)
DEFINE_GAMEPAD_BUTTON_HANDLE(rstick)
DEFINE_GAMEPAD_BUTTON_HANDLE(lshoulder)
DEFINE_GAMEPAD_BUTTON_HANDLE(rshoulder)
DEFINE_GAMEPAD_BUTTON_HANDLE(ltrigger)
DEFINE_GAMEPAD_BUTTON_HANDLE(rtrigger)
DEFINE_GAMEPAD_BUTTON_HANDLE(lgrip)
DEFINE_GAMEPAD_BUTTON_HANDLE(rgrip)

DEFINE_GAMEPAD_BUTTON_HANDLE(dpad_left)
DEFINE_GAMEPAD_BUTTON_HANDLE(dpad_right)
DEFINE_GAMEPAD_BUTTON_HANDLE(dpad_up)
DEFINE_GAMEPAD_BUTTON_HANDLE(dpad_down)

DEFINE_GAMEPAD_BUTTON_HANDLE(back)
DEFINE_GAMEPAD_BUTTON_HANDLE(guide)
DEFINE_GAMEPAD_BUTTON_HANDLE(start)

DEFINE_GAMEPAD_BUTTON_HANDLE(next)
DEFINE_GAMEPAD_BUTTON_HANDLE(previous)

DEFINE_GAMEPAD_BUTTON_HANDLE(face_a)
DEFINE_GAMEPAD_BUTTON_HANDLE(face_b)
DEFINE_GAMEPAD_BUTTON_HANDLE(face_c)
DEFINE_GAMEPAD_BUTTON_HANDLE(face_x)
DEFINE_GAMEPAD_BUTTON_HANDLE(face_y)
DEFINE_GAMEPAD_BUTTON_HANDLE(face_z)

DEFINE_GAMEPAD_BUTTON_HANDLE(face_1)
DEFINE_GAMEPAD_BUTTON_HANDLE(face_2)

DEFINE_GAMEPAD_BUTTON_HANDLE(trigger)
DEFINE_GAMEPAD_BUTTON_HANDLE(hat_up)
DEFINE_GAMEPAD_BUTTON_HANDLE(hat_down)
DEFINE_GAMEPAD_BUTTON_HANDLE(hat_left)
DEFINE_GAMEPAD_BUTTON_HANDLE(hat_right)

/**
 * Returns the ButtonHandle associated with the particular numbered joystick
 * button (zero-based), if there is one, or ButtonHandle::none() if there is
 * not.
 */
ButtonHandle GamepadButton::
joystick(int button_number) {
  if (button_number >= 0) {
    // "button1" does not exist, it is called "trigger" instead
    static pvector<ButtonHandle> buttons(1, _trigger);
    while ((size_t)button_number >= buttons.size()) {
      char numstr[20];
      sprintf(numstr, "joystick%d", (int)buttons.size() + 1);
      ButtonHandle handle;
      ButtonRegistry::ptr()->register_button(handle, numstr);
      buttons.push_back(handle);
    }
    return buttons[button_number];
  }
  return ButtonHandle::none();
}

/**
 * This is intended to be called only once, by the static initialization
 * performed in config_util.cxx.
 */
void GamepadButton::
init_gamepad_buttons() {
  ButtonRegistry::ptr()->register_button(_lstick, "lstick");
  ButtonRegistry::ptr()->register_button(_rstick, "rstick");
  ButtonRegistry::ptr()->register_button(_lshoulder, "lshoulder");
  ButtonRegistry::ptr()->register_button(_rshoulder, "rshoulder");
  ButtonRegistry::ptr()->register_button(_ltrigger, "ltrigger");
  ButtonRegistry::ptr()->register_button(_rtrigger, "rtrigger");
  ButtonRegistry::ptr()->register_button(_lgrip, "lgrip");
  ButtonRegistry::ptr()->register_button(_rgrip, "rgrip");

  ButtonRegistry::ptr()->register_button(_dpad_left, "dpad_left");
  ButtonRegistry::ptr()->register_button(_dpad_right, "dpad_right");
  ButtonRegistry::ptr()->register_button(_dpad_up, "dpad_up");
  ButtonRegistry::ptr()->register_button(_dpad_down, "dpad_down");

  ButtonRegistry::ptr()->register_button(_back, "back");
  ButtonRegistry::ptr()->register_button(_guide, "guide");
  ButtonRegistry::ptr()->register_button(_start, "start");

  ButtonRegistry::ptr()->register_button(_next, "next");
  ButtonRegistry::ptr()->register_button(_previous, "previous");

  ButtonRegistry::ptr()->register_button(_face_a, "face_a");
  ButtonRegistry::ptr()->register_button(_face_b, "face_b");
  ButtonRegistry::ptr()->register_button(_face_c, "face_c");
  ButtonRegistry::ptr()->register_button(_face_x, "face_x");
  ButtonRegistry::ptr()->register_button(_face_y, "face_y");
  ButtonRegistry::ptr()->register_button(_face_z, "face_z");

  ButtonRegistry::ptr()->register_button(_face_1, "face_1");
  ButtonRegistry::ptr()->register_button(_face_2, "face_2");

  ButtonRegistry::ptr()->register_button(_trigger, "trigger");
  ButtonRegistry::ptr()->register_button(_hat_up, "hat_up");
  ButtonRegistry::ptr()->register_button(_hat_down, "hat_down");
  ButtonRegistry::ptr()->register_button(_hat_left, "hat_left");
  ButtonRegistry::ptr()->register_button(_hat_right, "hat_right");
}
