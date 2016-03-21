/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_webgldisplay.cxx
 * @author rdb
 * @date 2015-04-01
 */

#include "config_webgldisplay.h"
#include "webGLGraphicsPipe.h"
#include "webGLGraphicsWindow.h"
#include "webGLGraphicsStateGuardian.h"
#include "graphicsPipeSelection.h"
#include "dconfig.h"
#include "pandaSystem.h"

Configure(config_webgldisplay);
NotifyCategoryDef(webgldisplay, "display");

ConfigureFn(config_webgldisplay) {
  init_libwebgldisplay();
}

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libwebgldisplay() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  WebGLGraphicsPipe::init_type();
  WebGLGraphicsWindow::init_type();
  WebGLGraphicsStateGuardian::init_type();

  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();
  selection->add_pipe_type(WebGLGraphicsPipe::get_class_type(),
                           WebGLGraphicsPipe::pipe_constructor);

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->set_system_tag("WebGL", "window_system", "HTML");
}
