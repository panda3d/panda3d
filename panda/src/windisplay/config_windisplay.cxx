// Filename: config_windisplay.cxx
// Created by:  drose (20Dec02)
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

#include "config_windisplay.h"
#include "winGraphicsPipe.h"
#include "winGraphicsWindow.h"
#include "dconfig.h"

Configure(config_windisplay);
NotifyCategoryDef(windisplay, "display");

ConfigureFn(config_windisplay) {
  init_libwindisplay();
}

ConfigVariableFilename icon_filename
("win32-window-icon", "");

ConfigVariableFilename color_cursor_filename
("win32-color-cursor", "");

ConfigVariableFilename mono_cursor_filename
("win32-mono-cursor", "");

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

ConfigVariableBool ime_composition_w
("ime-composition-w", true,
 PRC_DESC("For now, set this true to use the IME correctly on Win2000, or "
          "false on Win98.  This is temporary; once we have been able to "
          "verify that this distinction is actually necessary, we can replace "
          "this config variable with an actual OS detection."));

ConfigVariableBool ime_aware
("ime-aware", false,
 PRC_DESC("Set this true to show ime texts on the chat panel and hide the "
          "IME default windows. This is a mechanism to work around DX8/9 "
          "interface."));

ConfigVariableBool ime_hide
("ime-hide", false,
 PRC_DESC("Set this true to hide ime windows."));

ConfigVariableBool sync_video
("sync-video", true,
 PRC_DESC("Configure this true to force the rendering to sync to the video "
          "refresh, or false to let your frame rate go as high as it can, "
          "irrespective of the video refresh (if this capability is "
          "available in the ICD)."));

ConfigVariableBool swapbuffer_framelock
("swapbuffer-framelock", false,
 PRC_DESC("Set this true to enable HW swapbuffer frame-lock on 3dlabs cards"));

ConfigVariableBool force_software_renderer
("force-software-renderer", false);
ConfigVariableBool allow_software_renderer
("allow-software-renderer", false);

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
