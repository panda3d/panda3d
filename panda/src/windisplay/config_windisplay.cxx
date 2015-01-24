// Filename: config_windisplay.cxx
// Created by:  drose (20Dec02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "config_windisplay.h"
#include "winGraphicsPipe.h"
#include "winGraphicsWindow.h"
#include "dconfig.h"

Configure(config_windisplay);
NotifyCategoryDef(windisplay, "display");

ConfigureFn(config_windisplay) {
  init_libwindisplay();
}

ConfigVariableBool responsive_minimized_fullscreen_window
("responsive-minimized-fullscreen-window",false);

ConfigVariableBool hold_keys_across_windows
("hold-keys-across-windows", false,
 PRC_DESC("Set this true to remember the current state of the keyboard while "
          "the window focus is lost, or false to pretend the user is not "
          "holding down any keys while the window focus is lost.  In either "
          "case it should accurately restore the correct keyboard state when "
          "the window focus is regained."));

ConfigVariableBool do_vidmemsize_check
("do-vidmemsize-check", true,
 PRC_DESC("if true, use ddraw's GetAvailVidMem to fail if driver says "
          "it has too little video mem"));

ConfigVariableBool auto_cpu_data
("auto-cpu-data", false,
 PRC_DESC("Set this true to automatically get the CPU data at start; false to "
          "require an explicit call to pipe->lookup_cpu_data().  Setting this "
          "true may slow down startup time by 1-2 seconds."));

ConfigVariableBool ime_aware
("ime-aware", false,
 PRC_DESC("Set this true to show ime texts on the chat panel and hide the "
          "IME default windows. This is a mechanism to work around DX8/9 "
          "interface."));

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

ConfigVariableBool swapbuffer_framelock
("swapbuffer-framelock", false,
 PRC_DESC("Set this true to enable HW swapbuffer frame-lock on 3dlabs cards"));

////////////////////////////////////////////////////////////////////
//     Function: init_libwindisplay
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
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
