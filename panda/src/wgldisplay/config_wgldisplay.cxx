// Filename: config_wgldisplay.cxx
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

#include "config_wgldisplay.h"
#include "wglGraphicsPipe.h"
#include "wglGraphicsWindow.h"

#include <dconfig.h>

Configure(config_wgldisplay);
NotifyCategoryDef(wgldisplay, "display");

ConfigureFn(config_wgldisplay) {
  init_libwgldisplay();
}

//  Configure this true to force the rendering to sync to the video
//  refresh, or false to let your frame rate go as high as it can,
//  irrespective of the video refresh.  (if this capability is available in the ICD)
bool gl_sync_video = config_wgldisplay.GetBool("sync-video", true);

bool gl_show_fps_meter = config_wgldisplay.GetBool("show-fps-meter", false);
float gl_fps_meter_update_interval = max(0.5,config_wgldisplay.GetFloat("fps-meter-update-interval", 1.7));
int gl_forced_pixfmt=config_wgldisplay.GetInt("gl-force-pixfmt", 0);

bool bResponsive_minimized_fullscreen_window = config_wgldisplay.GetBool("responsive-minimized-fullscreen-window",false);

extern void AtExitFn(void);

////////////////////////////////////////////////////////////////////
//     Function: init_libwgldisplay
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libwgldisplay() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  wglGraphicsPipe::init_type();
  GraphicsPipe::get_factory().register_factory(
            wglGraphicsPipe::get_class_type(),
            wglGraphicsPipe::make_wglGraphicsPipe);
  wglGraphicsWindow::init_type();
  GraphicsWindow::get_factory().register_factory(
            wglGraphicsWindow::get_class_type(),
                wglGraphicsWindow::make_wglGraphicsWindow);

  atexit(AtExitFn);
}

// cant use global var cleanly because global var static init executed after init_libwgl(), incorrectly reiniting var
Filename get_icon_filename_() {
  string iconname = config_wgldisplay.GetString("win32-window-icon","");
  return ExecutionEnvironment::expand_string(iconname);
}

