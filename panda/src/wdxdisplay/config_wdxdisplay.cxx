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

#include "config_wdxdisplay.h"
#include "wdxGraphicsPipe.h"
#include "wdxGraphicsWindow.h"

#include <dconfig.h>

Configure(config_wdxdisplay);
NotifyCategoryDef(wdxdisplay, "display");

ConfigureFn(config_wdxdisplay) {
  init_libwdxdisplay();
}

int dx_use_multimon = config_wdxdisplay.GetInt("dx-multimon", 1);
bool dx_force_16bpp_zbuffer = config_wdxdisplay.GetBool("dx-force-16bpp-zbuffer", false);
bool bResponsive_minimized_fullscreen_window = config_wdxdisplay.GetBool("responsive-minimized-fullscreen-window",false);
bool dx_preserve_fpu_state = config_wdxdisplay.GetBool("dx-preserve-fpu-state", false);

extern void AtExitFn(void);

////////////////////////////////////////////////////////////////////
//     Function: init_libwdxdisplay
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libwdxdisplay() {
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
