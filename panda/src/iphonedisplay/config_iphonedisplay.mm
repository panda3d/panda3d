// Filename: config_iphonedisplay.cxx
// Created by:  drose (08Apr09)
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

#include "config_iphonedisplay.h"
#include "iPhoneGraphicsPipe.h"
#include "iPhoneGraphicsStateGuardian.h"
#include "iPhoneGraphicsWindow.h"

#include "graphicsPipeSelection.h"
#include "dconfig.h"
#include "pandaSystem.h"


Configure(config_iphonedisplay);

NotifyCategoryDef(iphonedisplay, "display");

ConfigureFn(config_iphonedisplay) {
  init_libiphonedisplay();
}

ConfigVariableBool iphone_autorotate_view
("iphone-autorotate-view", true,
 PRC_DESC("Set this true to enable the iphone application to rotate its "
          "view automatically according to the phone's orientation, or "
          "false for its view to remain fixed."));

////////////////////////////////////////////////////////////////////
//     Function: init_libiphonedisplay
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libiphonedisplay() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  IPhoneGraphicsPipe::init_type();
  IPhoneGraphicsWindow::init_type();
  IPhoneGraphicsStateGuardian::init_type();

  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();
  selection->add_pipe_type(IPhoneGraphicsPipe::get_class_type(), IPhoneGraphicsPipe::pipe_constructor);

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->set_system_tag("OpenGL", "window_system", "IPhone");
  ps->set_system_tag("OpenGL ES", "window_system", "IPhone");

  GLESinit_classes();
}

////////////////////////////////////////////////////////////////////
//     Function: get_pipe_type_iphonedisplay
//  Description: Returns the TypeHandle index of the recommended
//               graphics pipe type defined by this module.
////////////////////////////////////////////////////////////////////
int
get_pipe_type_iphonedisplay() {
  return IPhoneGraphicsPipe::get_class_type().get_index();
}
