// Filename: config_wgldisplay.cxx
// Created by:  mike (07Oct99)
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
}

// cant use global var cleanly because global var static init executed after init_libwgl(), incorrectly reiniting var
Filename get_icon_filename_() {
  string iconname = config_wgldisplay.GetString("win32-window-icon","");
  return ExecutionEnvironment::expand_string(iconname);
}

