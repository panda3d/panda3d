// Filename: config_wdxdisplay.cxx
// Created by:  mike (07Oct99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "config_wdxdisplay8.h"
#include "wdxGraphicsPipe8.h"
#include "wdxGraphicsWindow8.h"

#include <dconfig.h>

Configure(config_wdxdisplay);
NotifyCategoryDef(wdxdisplay, "display");

ConfigureFn(config_wdxdisplay) {
  init_libwdxdisplay8();
}

bool dx_force_16bpp_zbuffer = config_wdxdisplay.GetBool("dx-force-16bpp-zbuffer", false);
bool bResponsive_minimized_fullscreen_window = config_wdxdisplay.GetBool("responsive-minimized-fullscreen-window",false);
bool dx_preserve_fpu_state = config_wdxdisplay.GetBool("dx-preserve-fpu-state", false);
int dx_preferred_deviceID = config_wdxdisplay.GetInt("dx-preferred-device-id", -1);

// if true, use ddraw's GetAvailVidMem to fail if driver says it has too little video mem
bool dx_do_vidmemsize_check = config_wdxdisplay.GetBool("do-vidmemsize-check", true);

// For now, set this true to use the IME correctly on Win2000, or
// false on Win98.  This is temporary; once we have been able to
// verify that this distinction is actually necessary, we can replace
// this config variable with an actual OS detection.
bool ime_composition_w = config_wdxdisplay.GetBool("ime-composition-w", true);

extern void AtExitFn(void);

////////////////////////////////////////////////////////////////////
//     Function: init_libwdxdisplay
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void init_libwdxdisplay8() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  atexit(AtExitFn);

  wdxGraphicsPipe::init_type();
  GraphicsPipe::get_factory().register_factory(
            wdxGraphicsPipe::get_class_type(),
            wdxGraphicsPipe::make_wdxGraphicsPipe);
  wdxGraphicsWindow::init_type();
  GraphicsWindow::get_factory().register_factory(
            wdxGraphicsWindow::get_class_type(),
                wdxGraphicsWindow::make_wdxGraphicsWindow);

  set_global_parameters();
}

// cant use global var cleanly because global var static init executed after init_libwdxdisplay(), incorrectly reiniting var
Filename get_icon_filename() {
  string iconname = config_wdxdisplay.GetString("win32-window-icon","");
  return ExecutionEnvironment::expand_string(iconname);
}

Filename get_color_cursor_filename() {
  string cursorname = config_wdxdisplay.GetString("win32-color-cursor","");
  return ExecutionEnvironment::expand_string(cursorname);
}

Filename get_mono_cursor_filename() {
  string cursorname = config_wdxdisplay.GetString("win32-mono-cursor","");
  return ExecutionEnvironment::expand_string(cursorname);
}
