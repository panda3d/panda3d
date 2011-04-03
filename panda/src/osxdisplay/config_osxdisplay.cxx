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
#include <Carbon/Carbon.h>

#include "config_osxdisplay.h"
#include "osxGraphicsBuffer.h"
#include "osxGraphicsPipe.h"
#include "osxGraphicsStateGuardian.h"
#include "osxGraphicsWindow.h"

#include "graphicsPipeSelection.h"
#include "dconfig.h"
#include "pandaSystem.h"


Configure(config_osxdisplay);

NotifyCategoryDef(osxdisplay, "display");

ConfigureFn(config_osxdisplay) {
  init_libosxdisplay();
}

ConfigVariableBool show_resize_box
("show-resize-box", true,
 PRC_DESC("When this variable is true, then resizable OSX Panda windows will "
          "be rendered with a resize control in the lower-right corner.  "
          "This is specially handled by Panda, since otherwise the 3-d "
          "window would completely hide any resize control drawn by the "
          "OS.  Set this variable false to allow this control to be hidden."));

ConfigVariableBool osx_support_gl_buffer
("osx-support-gl-buffer", true,
 PRC_DESC("Set this true to support use of GLBuffers on OSX.  When true, there is a risk of "
          "a program crash due to buggy driver support for GLBuffers.  "
          "If this is false, offscreen buffers will be created as "
          "AGLPbuffers, which are less powerful, and appear to have their "
          "own set of problems."));

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

////////////////////////////////////////////////////////////////////
//     Function: init_libosxdisplay
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libosxdisplay() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  osxGraphicsBuffer::init_type();
  osxGraphicsPipe::init_type();
  osxGraphicsWindow::init_type();
  osxGraphicsStateGuardian::init_type();

  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();
  selection->add_pipe_type(osxGraphicsPipe::get_class_type(), osxGraphicsPipe::pipe_constructor);

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->set_system_tag("OpenGL", "window_system", "OSX");
}
