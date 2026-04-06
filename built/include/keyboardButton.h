/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file keyboardButton.h
 * @author drose
 * @date 2000-03-01
 */

#ifndef KEYBOARDBUTTON_H
#define KEYBOARDBUTTON_H

#include "pandabase.h"

#include "buttonHandle.h"

/**
 * This class is just used as a convenient namespace for grouping all of these
 * handy functions that return buttons which map to standard keyboard keys.
 */
class EXPCL_PANDA_PUTIL KeyboardButton {
PUBLISHED:
  static ButtonHandle ascii_key(char ascii_equivalent);

  static ButtonHandle space();
  static ButtonHandle backspace();
  static ButtonHandle tab();
  static ButtonHandle enter();
  static ButtonHandle escape();

  static ButtonHandle f1();
  static ButtonHandle f2();
  static ButtonHandle f3();
  static ButtonHandle f4();
  static ButtonHandle f5();
  static ButtonHandle f6();
  static ButtonHandle f7();
  static ButtonHandle f8();
  static ButtonHandle f9();
  static ButtonHandle f10();
  static ButtonHandle f11();
  static ButtonHandle f12();

  // PC keyboards don't have these four buttons, but Macs do.
  static ButtonHandle f13();
  static ButtonHandle f14();
  static ButtonHandle f15();
  static ButtonHandle f16();

  static ButtonHandle left();
  static ButtonHandle right();
  static ButtonHandle up();
  static ButtonHandle down();
  static ButtonHandle page_up();
  static ButtonHandle page_down();
  static ButtonHandle home();
  static ButtonHandle end();
  static ButtonHandle insert();
  static ButtonHandle del();  // delete is a C++ keyword.
  static ButtonHandle help();
  static ButtonHandle menu();

  static ButtonHandle shift();
  static ButtonHandle control();
  static ButtonHandle alt();
  static ButtonHandle meta();
  static ButtonHandle caps_lock();
  static ButtonHandle shift_lock();
  static ButtonHandle num_lock();
  static ButtonHandle scroll_lock();
  static ButtonHandle print_screen();
  static ButtonHandle pause();

  static ButtonHandle lshift();
  static ButtonHandle rshift();
  static ButtonHandle lcontrol();
  static ButtonHandle rcontrol();
  static ButtonHandle lalt();
  static ButtonHandle ralt();
  static ButtonHandle lmeta();
  static ButtonHandle rmeta();

public:
  static void init_keyboard_buttons();
};

#endif
