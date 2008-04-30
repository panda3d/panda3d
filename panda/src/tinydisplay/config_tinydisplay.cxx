// Filename: config_tinydisplay.cxx
// Created by:  drose (24Apr08)
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

#include "config_tinydisplay.h"
#include "tinyGraphicsPipe.h"
#include "tinyGraphicsWindow.h"
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

  TinyGraphicsPipe::init_type();
  TinyGraphicsWindow::init_type();
  TinyGraphicsStateGuardian::init_type();
  TinyGeomMunger::init_type();
  TinyTextureContext::init_type();

  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();
  selection->add_pipe_type(TinyGraphicsPipe::get_class_type(),
                           TinyGraphicsPipe::pipe_constructor);

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("TinyGL");
}
