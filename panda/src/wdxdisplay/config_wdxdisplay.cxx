// Filename: config_wdxdisplay.cxx
// Created by:  mike (07Oct99)
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

  wdxGraphicsPipe::init_type();
  GraphicsPipe::get_factory().register_factory(
            wdxGraphicsPipe::get_class_type(),
            wdxGraphicsPipe::make_wdxGraphicsPipe);
  wdxGraphicsWindow::init_type();
  GraphicsWindow::get_factory().register_factory(
            wdxGraphicsWindow::get_class_type(),
                wdxGraphicsWindow::make_wdxGraphicsWindow);
}

// cant use global var cleanly because global var static init executed after init_libwgl(), incorrectly reiniting var
Filename get_icon_filename() {
  string iconname = config_wdxdisplay.GetString("win32-window-icon","");
  return ExecutionEnvironment::expand_string(iconname);
}
