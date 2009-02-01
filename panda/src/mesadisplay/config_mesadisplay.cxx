// Filename: config_mesadisplay.cxx
// Created by:  drose (09Feb04)
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

////////////////////////////////////////////////////////////////////
//     Function: get_pipe_type_mesadisplay
//  Description: Returns the TypeHandle index of the recommended
//               graphics pipe type defined by this module.
////////////////////////////////////////////////////////////////////
int
get_pipe_type_mesadisplay() {
  return OsMesaGraphicsPipe::get_class_type().get_index();
}
