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

DEFINE_GAMEPAD_BUTTON_HANDLE(dpad_left)
DEFINE_GAMEPAD_BUTTON_HANDLE(dpad_right)
DEFINE_GAMEPAD_BUTTON_HANDLE(dpad_up)
DEFINE_GAMEPAD_BUTTON_HANDLE(dpad_down)

DEFINE_GAMEPAD_BUTTON_HANDLE(back)
DEFINE_GAMEPAD_BUTTON_HANDLE(guide)
DEFINE_GAMEPAD_BUTTON_HANDLE(start)

DEFINE_GAMEPAD_BUTTON_HANDLE(next)
DEFINE_GAMEPAD_BUTTON_HANDLE(previous)

DEFINE_GAMEPAD_BUTTON_HANDLE(action_a)
DEFINE_GAMEPAD_BUTTON_HANDLE(action_b)
DEFINE_GAMEPAD_BUTTON_HANDLE(action_c)
DEFINE_GAMEPAD_BUTTON_HANDLE(action_x)
DEFINE_GAMEPAD_BUTTON_HANDLE(action_y)
DEFINE_GAMEPAD_BUTTON_HANDLE(action_z)

DEFINE_GAMEPAD_BUTTON_HANDLE(action_1)
DEFINE_GAMEPAD_BUTTON_HANDLE(action_2)

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

  ButtonRegistry::ptr()->register_button(_dpad_left, "dpad_left");
  ButtonRegistry::ptr()->register_button(_dpad_right, "dpad_right");
  ButtonRegistry::ptr()->register_button(_dpad_up, "dpad_up");
  ButtonRegistry::ptr()->register_button(_dpad_down, "dpad_down");

  ButtonRegistry::ptr()->register_button(_back, "back");
  ButtonRegistry::ptr()->register_button(_guide, "guide");
  ButtonRegistry::ptr()->register_button(_start, "start");

  ButtonRegistry::ptr()->register_button(_next, "next");
  ButtonRegistry::ptr()->register_button(_previous, "previous");

  ButtonRegistry::ptr()->register_button(_action_a, "action_a");
  ButtonRegistry::ptr()->register_button(_action_b, "action_b");
  ButtonRegistry::ptr()->register_button(_action_c, "action_c");
  ButtonRegistry::ptr()->register_button(_action_x, "action_x");
  ButtonRegistry::ptr()->register_button(_action_y, "action_y");
  ButtonRegistry::ptr()->register_button(_action_z, "action_z");

  ButtonRegistry::ptr()->register_button(_action_1, "action_1");
  ButtonRegistry::ptr()->register_button(_action_2, "action_2");
}
