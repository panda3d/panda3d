// Filename: config_mesadisplay.cxx
// Created by:  drose (09Feb04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "config_mesadisplay.h"
#include "mesaGraphicsBuffer.h"
#include "mesaGraphicsPipe.h"
#include "mesaGraphicsStateGuardian.h"
#include "graphicsPipeSelection.h"
#include "dconfig.h"

Configure(config_mesadisplay);
NotifyCategoryDef(mesadisplay, "display");

ConfigureFn(config_mesadisplay) {
  init_libmesadisplay();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libmesadisplay
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libmesadisplay() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  MesaGraphicsBuffer::init_type();
  MesaGraphicsPipe::init_type();
  MesaGraphicsStateGuardian::init_type();

  cerr << "Adding MesaGraphicsPipe\n";
  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();
  selection->add_pipe_type(MesaGraphicsPipe::get_class_type(),
                           MesaGraphicsPipe::pipe_constructor);
}
