// Filename: config_wcrdisplay.cxx
// Created by:  skyler, based on wgl* file.
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

#include "config_wcrdisplay.h"
#include "wcrGraphicsPipe.h"
#include "wcrGraphicsWindow.h"

#include "dconfig.h"

Configure(config_wcrdisplay);
NotifyCategoryDef(wcrdisplay, "display");

ConfigureFn(config_wcrdisplay) {
  init_libwcrdisplay();
}

//  Configure this true to force the rendering to sync to the video
//  refresh, or false to let your frame rate go as high as it can,
//  irrespective of the video refresh.  (if this capability is available in the ICD)
bool gl_sync_video = config_wcrdisplay.GetBool("sync-video", true);

bool gl_show_fps_meter = config_wcrdisplay.GetBool("show-fps-meter", false);
float gl_fps_meter_update_interval = max(0.5,config_wcrdisplay.GetFloat("fps-meter-update-interval", 1.7));
int gl_forced_pixfmt=config_wcrdisplay.GetInt("gl-force-pixfmt", 0);

bool bResponsive_minimized_fullscreen_window = config_wcrdisplay.GetBool("responsive-minimized-fullscreen-window",false);

// Set this true to not attempt to use any of the function calls that
// will crab out WireGL.
bool support_wiregl = config_wcrdisplay.GetBool("support-wiregl", false);

extern void AtExitFn();

////////////////////////////////////////////////////////////////////
//     Function: init_libwcrdisplay
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libwcrdisplay() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  wcrGraphicsPipe::init_type();
  GraphicsPipe::get_factory().register_factory(
            wcrGraphicsPipe::get_class_type(),
            wcrGraphicsPipe::make_wcrGraphicsPipe);
  wcrGraphicsWindow::init_type();
  GraphicsWindow::get_factory().register_factory(
            wcrGraphicsWindow::get_class_type(),
                wcrGraphicsWindow::make_wcrGraphicsWindow);

  atexit(AtExitFn);
}

// cant use global var cleanly because global var static init executed after init_libwcr(), incorrectly reiniting var
Filename get_icon_filename_2() {
  string iconname = config_wcrdisplay.GetString("win32-window-icon","");
  return ExecutionEnvironment::expand_string(iconname);
}

Filename get_color_cursor_filename_2() {
  string cursorname = config_wcrdisplay.GetString("win32-color-cursor","");
  return ExecutionEnvironment::expand_string(cursorname);
}

Filename get_mono_cursor_filename_2() {
  string cursorname = config_wcrdisplay.GetString("win32-mono-cursor","");
  return ExecutionEnvironment::expand_string(cursorname);
}
