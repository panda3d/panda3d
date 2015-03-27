// Filename: config_cocoadisplay.cxx
// Created by:  rdb (17May12)
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

#include "config_cocoadisplay.h"
#include "cocoaGraphicsPipe.h"
#include "cocoaGraphicsStateGuardian.h"
#include "cocoaGraphicsWindow.h"
#include "graphicsPipeSelection.h"
#include "dconfig.h"
#include "pandaSystem.h"

Configure(config_cocoadisplay);
NotifyCategoryDef(cocoadisplay, "display");

ConfigureFn(config_cocoadisplay) {
  init_libcocoadisplay();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libcocoadisplay
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libcocoadisplay() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  CocoaGraphicsPipe::init_type();
  CocoaGraphicsStateGuardian::init_type();
  CocoaGraphicsWindow::init_type();

  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();
  selection->add_pipe_type(CocoaGraphicsPipe::get_class_type(),
                           CocoaGraphicsPipe::pipe_constructor);

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->set_system_tag("OpenGL", "window_system", "Cocoa");
}
