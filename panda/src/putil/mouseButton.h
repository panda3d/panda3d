// Filename: mouseButton.h
// Created by:  drose (01Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef MOUSEBUTTON_H
#define MOUSEBUTTON_H

#include <pandabase.h>

#include "buttonHandle.h"

////////////////////////////////////////////////////////////////////
// 	 Class : MouseButton
// Description : This class is just used as a convenient namespace for
//               grouping all of these handy functions that return
//               buttons which map to standard mouse buttons.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA MouseButton {
public:
  static ButtonHandle button(int button_number);
  static ButtonHandle one();
  static ButtonHandle two();
  static ButtonHandle three();

  static void init_mouse_buttons();
};

#endif
