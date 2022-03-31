/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_x11display.cxx
 * @author rdb
 * @date 2009-07-07
 */

#include "config_x11display.h"
#include "x11GraphicsPipe.h"
#include "x11GraphicsWindow.h"
#include "graphicsPipeSelection.h"
#include "dconfig.h"
#include "pandaSystem.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDAX11)
  #error Buildsystem error: BUILDING_PANDAX11 not defined
#endif

Configure(config_x11display);
NotifyCategoryDef(x11display, "display");

ConfigureFn(config_x11display) {
  init_libx11display();
}

ConfigVariableString display_cfg
("display", "",
 PRC_DESC("Specify the X display string for the default display.  If this "
          "is not specified, $DISPLAY is used."));

ConfigVariableBool x_error_abort
("x-error-abort", false,
 PRC_DESC("Set this true to trigger and abort (and a stack trace) on receipt "
          "of an error from the X window system.  This can make it easier "
          "to discover where these errors are generated."));

ConfigVariableBool x_init_threads
("x-init-threads", false,
 PRC_DESC("Set this true to ask Panda3D to call XInitThreads() upon loading "
          "the display module, which may help with some threading issues."));

ConfigVariableInt x_wheel_up_button
("x-wheel-up-button", 4,
 PRC_DESC("This is the mouse button index of the wheel_up event: which "
          "mouse button number does the system report when the mouse wheel "
          "is rolled one notch up?"));

ConfigVariableInt x_wheel_down_button
("x-wheel-down-button", 5,
 PRC_DESC("This is the mouse button index of the wheel_down event: which "
          "mouse button number does the system report when the mouse wheel "
          "is rolled one notch down?"));

ConfigVariableInt x_wheel_left_button
("x-wheel-left-button", 6,
 PRC_DESC("This is the mouse button index of the wheel_left event: which "
          "mouse button number does the system report when one scrolls "
          "to the left?"));

ConfigVariableInt x_wheel_right_button
("x-wheel-right-button", 7,
 PRC_DESC("This is the mouse button index of the wheel_right event: which "
          "mouse button number does the system report when one scrolls "
          "to the right?"));

ConfigVariableInt x_cursor_size
("x-cursor-size", -1,
 PRC_DESC("This sets the cursor size when using XCursor to change the mouse "
          "cursor.  The default is to use the default size for the display."));

ConfigVariableString x_wm_class_name
("x-wm-class-name", "",
 PRC_DESC("Specify the value to use for the res_name field of the window's "
          "WM_CLASS property.  Has no effect when x-wm-class is not set."));

ConfigVariableString x_wm_class
("x-wm-class", "",
 PRC_DESC("Specify the value to use for the res_class field of the window's "
          "WM_CLASS property."));

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libx11display() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  x11GraphicsPipe::init_type();
  x11GraphicsWindow::init_type();
}
