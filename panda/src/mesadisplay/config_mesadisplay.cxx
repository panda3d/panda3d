// Filename: config_mesadisplay.cxx
// Created by:  drose (09Feb04)
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

#include "config_mesadisplay.h"
#include "osMesaGraphicsBuffer.h"
#include "osMesaGraphicsPipe.h"
#include "osMesaGraphicsStateGuardian.h"
#include "graphicsPipeSelection.h"
#include "dconfig.h"
#include "mesagsg.h"

ConfigureDef(config_mesadisplay);
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

  OsMesaGraphicsBuffer::init_type();
  OsMesaGraphicsPipe::init_type();
  OSMesaGraphicsStateGuardian::init_type();

  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();
  selection->add_pipe_type(OsMesaGraphicsPipe::get_class_type(),
                           OsMesaGraphicsPipe::pipe_constructor);

  Mesainit_classes();
}
