/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_cocoadisplay.mm
 * @author rdb
 * @date 2012-05-17
 */

#include "config_cocoadisplay.h"
#include "cocoaGraphicsPipe.h"
#include "cocoaGraphicsWindow.h"
#include "dconfig.h"
#include "pandaSystem.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_COCOADISPLAY)
  #error Buildsystem error: BUILDING_PANDA_COCOADISPLAY not defined
#endif

Configure(config_cocoadisplay);
NotifyCategoryDef(cocoadisplay, "display");

ConfigureFn(config_cocoadisplay) {
  init_libcocoadisplay();
}

ConfigVariableBool cocoa_invert_wheel_x
("cocoa-invert-wheel-x", false,
 PRC_DESC("Set this to true to swap the wheel_left and wheel_right mouse "
          "button events, to restore to the pre-1.10.12 behavior."));
ConfigVariableBool dpi_aware
("dpi-aware", false,
 PRC_DESC("The default behavior on macOS is for Panda3D to use upscaling on"
          "high DPI screen. Set this to true to let the application use the"
          "actual pixel density of the screen."));

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libcocoadisplay() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  CocoaGraphicsPipe::init_type();
  CocoaGraphicsWindow::init_type();
}
