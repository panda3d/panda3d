// Filename: config_glxdisplay.cxx
// Created by:  cary (07Oct99)
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

NotifyCategoryDef( osxdisplay , "display");

ConfigureFn(config_osxdisplay) {
  init_libosxdisplay();
}

ConfigVariableString display_cfg
("display", "");

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

  printf("****** In libosx  init \n");

  osxGraphicsStateGuardian::init_type();
  osxGraphicsPipe::init_type();
  osxGraphicsWindow::init_type();
  osxGraphicsStateGuardian::init_type();



  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();
  selection->add_pipe_type(osxGraphicsPipe::get_class_type(), osxGraphicsPipe::pipe_constructor);

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->set_system_tag("OpenGL", "window_system", "OSX");



  printf("****** out libosx  init \n");
}
