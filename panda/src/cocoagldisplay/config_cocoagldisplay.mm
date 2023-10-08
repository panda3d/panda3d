/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_cocoagldisplay.mm
 * @author rdb
 * @date 2023-03-20
 */

#include "config_cocoagldisplay.h"
#include "cocoaGLGraphicsBuffer.h"
#include "cocoaGLGraphicsPipe.h"
#include "cocoaGLGraphicsStateGuardian.h"
#include "cocoaGLGraphicsWindow.h"
#include "graphicsPipeSelection.h"
#include "dconfig.h"
#include "pandaSystem.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_COCOAGLDISPLAY)
  #error Buildsystem error: BUILDING_PANDA_COCOAGLDISPLAY not defined
#endif

Configure(config_cocoagldisplay);

ConfigureFn(config_cocoagldisplay) {
  init_libcocoagldisplay();
}

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libcocoagldisplay() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  init_libcocoadisplay();

  CocoaGLGraphicsBuffer::init_type();
  CocoaGLGraphicsPipe::init_type();
  CocoaGLGraphicsStateGuardian::init_type();
  CocoaGLGraphicsWindow::init_type();

  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();
  selection->add_pipe_type(CocoaGLGraphicsPipe::get_class_type(),
                           CocoaGLGraphicsPipe::pipe_constructor);

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->set_system_tag("OpenGL", "window_system", "Cocoa");
}
