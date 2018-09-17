/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_wgldisplay.cxx
 * @author drose
 * @date 2002-12-20
 */

#include "config_wgldisplay.h"
#include "wglGraphicsBuffer.h"
#include "wglGraphicsPipe.h"
#include "wglGraphicsStateGuardian.h"
#include "wglGraphicsWindow.h"
#include "graphicsPipeSelection.h"
#include "dconfig.h"
#include "pandaSystem.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_WGLDISPLAY)
  #error Buildsystem error: BUILDING_PANDA_WGLDISPLAY not defined
#endif

Configure(config_wgldisplay);
NotifyCategoryDef(wgldisplay, "display");

ConfigureFn(config_wgldisplay) {
  init_libwgldisplay();
}

ConfigVariableInt gl_force_pixfmt
("gl-force-pixfmt", 0);

ConfigVariableBool gl_force_invalid
("gl-force-invalid", false,
 PRC_DESC("Set this true to force all GL windows to fail to open "
          "correctly (for debugging)."));

ConfigVariableBool gl_do_vidmemsize_check
("gl-do-vidmemsize-check", true,
 PRC_DESC("This is true to insist that low-memory cards open only 640x480 "
          "fullscreen windows, no matter what resolution of window was "
          "requested.  It only affects fullscreen windows."));

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libwgldisplay() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  wglGraphicsBuffer::init_type();
  wglGraphicsPipe::init_type();
  wglGraphicsStateGuardian::init_type();
  wglGraphicsWindow::init_type();

  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();
  selection->add_pipe_type(wglGraphicsPipe::get_class_type(),
                           wglGraphicsPipe::pipe_constructor);

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->set_system_tag("OpenGL", "window_system", "WGL");
}
