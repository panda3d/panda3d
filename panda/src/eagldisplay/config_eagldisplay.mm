/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_eagldisplay.mm
 * @author D. Lawrence
 * @date 2019-01-03
 */

#include "config_eagldisplay.h"
#include "dconfig.h"
#include "graphicsPipeSelection.h"
#include "pandaSystem.h"
#include "eaglGraphicsPipe.h"
#include "eaglGraphicsWindow.h"
#include "eaglGraphicsStateGuardian.h"
#include <dispatch/dispatch.h>

Configure(config_eagldisplay);

NotifyCategoryDef(eagldisplay, "display");

ConfigureFn(config_eagldisplay) {
  init_libeagldisplay();
}

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libeagldisplay() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;
  
  EAGLGraphicsPipe::init_type();
  EAGLGraphicsWindow::init_type();
  EAGLGraphicsStateGuardian::init_type();
  
  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();
  selection->add_pipe_type(EAGLGraphicsPipe::get_class_type(), EAGLGraphicsPipe::pipe_constructor);
  
  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->set_system_tag("OpenGL ES 2", "window_system", "iOS");
}
