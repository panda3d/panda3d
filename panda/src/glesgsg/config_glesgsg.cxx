// Filename: config_glesgsg.cxx
// Created by:  pro-rsoft (21May09)
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

#include "config_glesgsg.h"
#include "glesgsg.h"

#include "dconfig.h"

ConfigureDef(config_glesgsg);
NotifyCategoryDef(glesgsg, ":display:gsg");

ConfigureFn(config_glesgsg) {
  init_libglesgsg();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libglesgsg
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libglesgsg() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  GLESinit_classes();
}
