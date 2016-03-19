/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file gamepadButton.h
 * @author rdb
 * @date 2015-08-21
 */

#ifndef GAMEPADBUTTON_H
#define GAMEPADBUTTON_H

#include "pandabase.h"

#include "buttonHandle.h"

/**
 * This class is just used as a convenient namespace for grouping all of these
 * handy functions that return buttons which map to gamepad buttons.
 */
class EXPCL_PANDA_PUTIL GamepadButton {
PUBLISHED:
  static ButtonHandle lstick();
  static ButtonHandle rstick();
  static ButtonHandle lshoulder();
  static ButtonHandle rshoulder();
  static ButtonHandle ltrigger();
  static ButtonHandle rtrigger();

  static ButtonHandle dpad_left();
  static ButtonHandle dpad_right();
  static ButtonHandle dpad_up();
  static ButtonHandle dpad_down();

  static ButtonHandle back();
  static ButtonHandle guide();
  static ButtonHandle start();

  static ButtonHandle action_a();
  static ButtonHandle action_b();
  static ButtonHandle action_c();
  static ButtonHandle action_x();
  static ButtonHandle action_y();
  static ButtonHandle action_z();

public:
  static void init_gamepad_buttons();
};

#endif
