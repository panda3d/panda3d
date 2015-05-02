// Filename: config_tinydisplay.cxx
// Created by:  drose (24Apr08)
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

#include "config_tinydisplay.h"
#include "tinyXGraphicsPipe.h"
#include "tinyXGraphicsWindow.h"
#include "tinyWinGraphicsPipe.h"
#include "tinyWinGraphicsWindow.h"
#include "tinyOsxGraphicsPipe.h"
#include "tinyOsxGraphicsWindow.h"
#include "tinySDLGraphicsPipe.h"
#include "tinySDLGraphicsWindow.h"
#include "tinyOffscreenGraphicsPipe.h"
#include "tinyGraphicsBuffer.h"
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

ConfigVariableBool td_ignore_mipmaps
  ("td-ignore-mipmaps", false,
   PRC_DESC("Configure this true to disable use of mipmaps on the "
            "tinydisplay software renderer."));

ConfigVariableBool td_ignore_clamp
  ("td-ignore-clamp", false,
   PRC_DESC("Configure this true to disable texture clamp mode and other "
            "wrap modes other than repeat (all textures will repeat, which "
            "is a little cheaper)."));

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

  TinyGraphicsBuffer::init_type();
  TinyGraphicsStateGuardian::init_type();
  TinyGeomMunger::init_type();
  TinyTextureContext::init_type();

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("TinyPanda");

  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();

#ifdef HAVE_X11
  TinyXGraphicsPipe::init_type();
  TinyXGraphicsWindow::init_type();
  selection->add_pipe_type(TinyXGraphicsPipe::get_class_type(),
                           TinyXGraphicsPipe::pipe_constructor);
  ps->set_system_tag("TinyPanda", "native_window_system", "X11");
#endif

#ifdef WIN32
  TinyWinGraphicsPipe::init_type();
  TinyWinGraphicsWindow::init_type();
  selection->add_pipe_type(TinyWinGraphicsPipe::get_class_type(),
                           TinyWinGraphicsPipe::pipe_constructor);
  ps->set_system_tag("TinyPanda", "native_window_system", "Win");
#endif

#if defined(IS_OSX) && !defined(BUILD_IPHONE) && defined(HAVE_CARBON) && !__LP64__
  TinyOsxGraphicsPipe::init_type();
  TinyOsxGraphicsWindow::init_type();
  selection->add_pipe_type(TinyOsxGraphicsPipe::get_class_type(),
                           TinyOsxGraphicsPipe::pipe_constructor);
  ps->set_system_tag("TinyPanda", "native_window_system", "OSX");
#endif

#ifdef HAVE_SDL
  TinySDLGraphicsPipe::init_type();
  TinySDLGraphicsWindow::init_type();
  selection->add_pipe_type(TinySDLGraphicsPipe::get_class_type(),
                           TinySDLGraphicsPipe::pipe_constructor);
  ps->set_system_tag("TinyPanda", "SDL", "SDL");
#endif

  TinyOffscreenGraphicsPipe::init_type();
  selection->add_pipe_type(TinyOffscreenGraphicsPipe::get_class_type(),
                           TinyOffscreenGraphicsPipe::pipe_constructor);
  ps->set_system_tag("TinyPanda", "", "");
}

////////////////////////////////////////////////////////////////////
//     Function: get_pipe_type_p3tinydisplay
//  Description: Returns the TypeHandle index of the recommended
//               graphics pipe type defined by this module.
////////////////////////////////////////////////////////////////////
int
get_pipe_type_p3tinydisplay() {

#ifdef WIN32
  return TinyWinGraphicsPipe::get_class_type().get_index();
#endif

#if defined(IS_OSX) && !defined(BUILD_IPHONE) && defined(HAVE_CARBON) && !__LP64__
  return TinyOsxGraphicsPipe::get_class_type().get_index();
#endif

#ifdef HAVE_X11
  return TinyXGraphicsPipe::get_class_type().get_index();
#endif

#ifdef HAVE_SDL
  return TinySDLGraphicsPipe::get_class_type().get_index();
#endif

  return TinyOffscreenGraphicsPipe::get_class_type().get_index();
}
