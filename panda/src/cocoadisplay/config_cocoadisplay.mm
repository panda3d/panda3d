/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_cocoadisplay.mm
 * @author rdb
 * @date 2012-05-17
 */

#include "config_cocoadisplay.h"
#include "cocoaGraphicsBuffer.h"
#include "cocoaGraphicsPipe.h"
#include "cocoaGraphicsStateGuardian.h"
#include "cocoaGraphicsWindow.h"
#include "graphicsPipeSelection.h"
#include "dconfig.h"
#include "pandaSystem.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_COCOADISPLAY)
  #error Buildsystem error: BUILDING_PANDA_COCOADISPLAY not defined
#endif

Configure(config_cocoadisplay);
NotifyCategoryDef(cocoadisplay, "display");

ConfigureFn(config_cocoadisplay) {
  init_libcocoadisplay();
}

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libcocoadisplay() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  CocoaGraphicsBuffer::init_type();
  CocoaGraphicsPipe::init_type();
  CocoaGraphicsStateGuardian::init_type();
  CocoaGraphicsWindow::init_type();

  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();
  selection->add_pipe_type(CocoaGraphicsPipe::get_class_type(),
                           CocoaGraphicsPipe::pipe_constructor);

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->set_system_tag("OpenGL", "window_system", "Cocoa");
}
