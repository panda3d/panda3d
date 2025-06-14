/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_windisplay.cxx
 * @author drose
 * @date 2002-12-20
 */

#include "config_windisplay.h"
#include "winGraphicsPipe.h"
#include "winGraphicsWindow.h"
#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDAWIN)
  #error Buildsystem error: BUILDING_PANDAWIN not defined
#endif

Configure(config_windisplay);
NotifyCategoryDef(windisplay, "display");

ConfigureFn(config_windisplay) {
  init_libwindisplay();
}

ConfigVariableBool do_vidmemsize_check
("do-vidmemsize-check", true,
 PRC_DESC("if true, use ddraw's GetAvailVidMem to fail if driver says "
          "it has too little video mem"));

ConfigVariableBool auto_cpu_data
("auto-cpu-data", false,
 PRC_DESC("Set this true to automatically get the CPU data at start; false to "
          "require an explicit call to pipe->lookup_cpu_data().  Setting this "
          "true may slow down startup time by 1-2 seconds."));

ConfigVariableBool ime_hide
("ime-hide", false,
 PRC_DESC("Set this true to hide ime windows."));

ConfigVariableBool request_dxdisplay_information
("request-dxdisplay-information", false,
 PRC_DESC("Setting this to true enables lumberingly slow and evil code at "
          "start-up that creates a Direct3D window and subsequently fills up "
          "up the video memory with dummy textures in order to find out how "
          "much video memory there actually is.  Leave this disabled unless "
          "you have a specific need for this information and don't mind "
          "having a slow start-up."));

ConfigVariableBool dpi_aware
("dpi-aware", true,
 PRC_DESC("The default behavior is for Panda3D to disable DPI-virtualization "
          "that is introduced in Windows 8.1.  Set this to false if you are "
          "experiencing problems with this setting."));

ConfigVariableBool dpi_window_resize
("dpi-window-resize", false,
 PRC_DESC("Set this to true to let Panda3D resize the window according to the "
          "DPI settings whenever the window is dragged to a monitor with "
          "different DPI, or when the DPI setting is changed in the control "
          "panel.  Only available in Windows 8.1 and later, and requires "
          "dpi-aware to be set as well."));

ConfigVariableBool paste_emit_keystrokes
("paste-emit-keystrokes", true,
 PRC_DESC("Handle paste events (Ctrl-V) as separate keystroke events for each "
          "pasted character."));

ConfigVariableBool disable_message_loop
("disable-message-loop", false,
 PRC_DESC("If this is false, Panda will process messages from the Windows "
          "message loop, which is required for normal operation.  You may set "
          "this to true if some other UI framework (such as Tcl/Tk) needs "
          "exclusive ownership of the message loop."));

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libwindisplay() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  WinGraphicsPipe::init_type();
  WinGraphicsWindow::init_type();
}
