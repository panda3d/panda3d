// Filename: mouseButton.h
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

#ifndef MOUSEBUTTON_H
#define MOUSEBUTTON_H

#include "pandabase.h"

#include "buttonHandle.h"

////////////////////////////////////////////////////////////////////
//       Class : MouseButton
// Description : This class is just used as a convenient namespace for
//               grouping all of these handy functions that return
//               buttons which map to standard mouse buttons.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA MouseButton {
PUBLISHED:
  static ButtonHandle button(int button_number);
  static ButtonHandle one();
  static ButtonHandle two();
  static ButtonHandle three();
  static ButtonHandle four();
  static ButtonHandle five();
  static ButtonHandle wheel_up();
  static ButtonHandle wheel_down();

  static bool is_mouse_button(ButtonHandle button);

public:
  static void init_mouse_buttons();

  enum { num_mouse_buttons = 5 };
  static ButtonHandle _buttons[num_mouse_buttons];
  static ButtonHandle _wheel_up;
  static ButtonHandle _wheel_down;
};

#endif
