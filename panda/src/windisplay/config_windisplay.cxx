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

bool show_fps_meter = config_windisplay.GetBool("show-fps-meter", false);
float fps_meter_update_interval = max(0.5,config_windisplay.GetFloat("fps-meter-update-interval", 1.7));

bool responsive_minimized_fullscreen_window = config_windisplay.GetBool("responsive-minimized-fullscreen-window",false);

// Set this true to remember the current state of the keyboard while
// the window focus is lost, or false to pretend the user is not
// holding down any keys while the window focus is lost.  In either
// case it should accurately restore the correct keyboard state when
// the window focus is regained.
bool hold_keys_across_windows = config_windisplay.GetBool("hold-keys-across-windows", false);

// if true, use ddraw's GetAvailVidMem to fail if driver says it has too little video mem
bool do_vidmemsize_check = config_windisplay.GetBool("do-vidmemsize-check", true);

// For now, set this true to use the IME correctly on Win2000, or
// false on Win98.  This is temporary; once we have been able to
// verify that this distinction is actually necessary, we can replace
// this config variable with an actual OS detection.
bool ime_composition_w = config_windisplay.GetBool("ime-composition-w", true);

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

// cant use global var cleanly because global var static init executed
// after init_libwin(), incorrectly reiniting var
Filename get_icon_filename() {
  string iconname = config_windisplay.GetString("win32-window-icon","");
  return ExecutionEnvironment::expand_string(iconname);
}

Filename get_color_cursor_filename() {
  string cursorname = config_windisplay.GetString("win32-color-cursor","");
  return ExecutionEnvironment::expand_string(cursorname);
}

Filename get_mono_cursor_filename() {
  string cursorname = config_windisplay.GetString("win32-mono-cursor","");
  return ExecutionEnvironment::expand_string(cursorname);
}


//  Configure this true to force the rendering to sync to the video
//  refresh, or false to let your frame rate go as high as it can,
//  irrespective of the video refresh.  (if this capability is
//  available in the ICD)
bool sync_video = config_windisplay.GetBool("sync-video", true);

// Set this true to enable HW swapbuffer frame-lock on 3dlabs cards
bool swapbuffer_framelock = config_windisplay.GetBool("swapbuffer-framelock", false);

bool force_software_renderer = config_windisplay.GetBool("force-software-renderer", false);
bool allow_software_renderer = config_windisplay.GetBool("allow-software-renderer", false);
