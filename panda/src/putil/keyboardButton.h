// Filename: keyboardButton.h
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

#ifndef KEYBOARDBUTTON_H
#define KEYBOARDBUTTON_H

#include "pandabase.h"

#include "buttonHandle.h"

////////////////////////////////////////////////////////////////////
//       Class : KeyboardButton
// Description : This class is just used as a convenient namespace for
//               grouping all of these handy functions that return
//               buttons which map to standard keyboard keys.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA KeyboardButton {
PUBLISHED:
  static ButtonHandle ascii_key(char ascii_equivalent);
  static ButtonHandle ascii_key(const string &ascii_equivalent);

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

  static ButtonHandle shift();
  static ButtonHandle control();
  static ButtonHandle alt();
  static ButtonHandle meta();
  static ButtonHandle caps_lock();
  static ButtonHandle shift_lock();
  static ButtonHandle num_lock();
  static ButtonHandle scroll_lock();
  static ButtonHandle print_screen();

  static ButtonHandle lshift();
  static ButtonHandle rshift();
  static ButtonHandle lcontrol();
  static ButtonHandle rcontrol();
  static ButtonHandle lalt();
  static ButtonHandle ralt();

public:
  static void init_keyboard_buttons();
};

#endif
