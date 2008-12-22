// Filename: config_daeegg.cxx
// Created by:  pro-rsoft (30Oct08)
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

#include "config_daeegg.h"
#include "daeCharacter.h"
#include "daeMaterials.h"

#include "dconfig.h"

Configure(config_daeegg);
NotifyCategoryDef(daeegg, "");

ConfigureFn(config_daeegg) {
  init_libdaeegg();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libdaeegg
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libdaeegg() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;
  
  DaeCharacter::init_type();
  DaeMaterials::init_type();
}

