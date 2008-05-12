// Filename: config_tinydisplay.cxx
// Created by:  drose (24Apr08)
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

#include "config_tinydisplay.h"
#include "tinyXGraphicsPipe.h"
#include "tinyXGraphicsWindow.h"
#include "tinyWinGraphicsPipe.h"
#include "tinyWinGraphicsWindow.h"
#include "tinyOsxGraphicsPipe.h"
#include "tinyOsxGraphicsWindow.h"
#include "tinySDLGraphicsPipe.h"
#include "tinySDLGraphicsWindow.h"
#include "tinyGraphicsStateGuardian.h"
#include "tinyGeomMunger.h"
#include "tinyTextureContext.h"
#include "graphicsPipeSelection.h"
#include "dconfig.h"
#include "pandaSystem.h"

Configure(config_tinydisplay);
NotifyCategoryDef(tinydisplay, "display");

ConfigureFn(config_tinydisplay) {
  init_libtinydisplay();
}

ConfigVariableString display_cfg
("display", "",
 PRC_DESC("Specify the X display string for the default display.  If this "
          "is not specified, $DISPLAY is used."));

ConfigVariableBool x_error_abort
("x-error-abort", false,
 PRC_DESC("Set this true to trigger and abort (and a stack trace) on receipt "
          "of an error from the X window system.  This can make it easier "
          "to discover where these errors are generated."));

ConfigVariableInt x_wheel_up_button
("x-wheel-up-button", 4,
 PRC_DESC("This is the mouse button index of the wheel_up event: which "
          "mouse button number does the system report when the mouse wheel "
          "is rolled one notch up?"));

ConfigVariableInt x_wheel_down_button
("x-wheel-down-button", 5,
 PRC_DESC("This is the mouse button index of the wheel_down event: which "
          "mouse button number does the system report when the mouse wheel "
          "is rolled one notch down?"));

ConfigVariableBool show_resize_box
("show-resize-box", true,
 PRC_DESC("When this variable is true, then resizable OSX Panda windows will "
          "be rendered with a resize control in the lower-right corner.  "
          "This is specially handled by Panda, since otherwise the 3-d "
          "window would completely hide any resize control drawn by the "
          "OS.  Set this variable false to allow this control to be hidden."));

ConfigVariableBool osx_disable_event_loop
("osx-disable-event-loop", false,
 PRC_DESC("Set this true to disable the window event loop for the Panda "
          "windows.  This makes sense only in a publish environment where "
          "the window event loop is already handled by another part of the "
          "app."));

ConfigVariableInt osx_mouse_wheel_scale
("osx-mouse-wheel-scale", 1,
 PRC_DESC("Specify the number of units to spin the Mac mouse wheel to "
          "represent a single wheel_up or wheel_down message."));

ConfigVariableInt td_texture_ram
("td-texture-ram", -1,
 PRC_DESC("This specifies the maximum amount of RAM to devote to keeping "
          "textures resident with the tinydisplay software renderer.  When "
          "this limit is exceeded, textures over the limit that have not "
          "been rendered within the current frame will be evicted.  "
          "(Textures will not be evicted while they are still in the "
          "frame, even if this means this limit remains exceeded.)  "
          "Set it to -1 for no limit."));

ConfigVariableBool td_ignore_mipmaps
  ("td-ignore-mipmaps", false,
   PRC_DESC("Configure this true to disable use of mipmaps on the "
            "tinydisplay software renderer."));

ConfigVariableBool td_perspective_textures
  ("td-perspective-textures", true,
   PRC_DESC("Configure this false to disable use of perspective-correct "
            "textures on the tinydisplay software renderer, for a small "
            "performance gain."));

////////////////////////////////////////////////////////////////////
//     Function: init_libtinydisplay
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libtinydisplay() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  TinyGraphicsStateGuardian::init_type();
  TinyGeomMunger::init_type();
  TinyTextureContext::init_type();

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("TinyGL");

  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();

#ifdef IS_LINUX
  TinyXGraphicsPipe::init_type();
  TinyXGraphicsWindow::init_type();
  selection->add_pipe_type(TinyXGraphicsPipe::get_class_type(),
                           TinyXGraphicsPipe::pipe_constructor);
  ps->set_system_tag("TinyGL", "native_window_system", "X");
#endif

#ifdef WIN32
  TinyWinGraphicsPipe::init_type();
  TinyWinGraphicsWindow::init_type();
  selection->add_pipe_type(TinyWinGraphicsPipe::get_class_type(),
                           TinyWinGraphicsPipe::pipe_constructor);
  ps->set_system_tag("TinyGL", "native_window_system", "Win");
#endif

#ifdef IS_OSX
  TinyOsxGraphicsPipe::init_type();
  TinyOsxGraphicsWindow::init_type();
  selection->add_pipe_type(TinyOsxGraphicsPipe::get_class_type(),
                           TinyOsxGraphicsPipe::pipe_constructor);
  ps->set_system_tag("TinyGL", "OSX", "OSX");
#endif

#ifdef HAVE_SDL
  TinySDLGraphicsPipe::init_type();
  TinySDLGraphicsWindow::init_type();
  selection->add_pipe_type(TinySDLGraphicsPipe::get_class_type(),
                           TinySDLGraphicsPipe::pipe_constructor);
  ps->set_system_tag("TinyGL", "SDL", "SDL");
#endif
}
