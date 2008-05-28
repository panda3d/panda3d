// Filename: config_glgsg.cxx
// Created by:  drose (06Oct99)
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

#include "config_glgsg.h"
#include "glgsg.h"

#include "dconfig.h"

ConfigureDef(config_glgsg);
NotifyCategoryDef(glgsg, ":display:gsg");

ConfigureFn(config_glgsg) {
  init_libglgsg();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libglgsg
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libglgsg() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  GLinit_classes();
}
