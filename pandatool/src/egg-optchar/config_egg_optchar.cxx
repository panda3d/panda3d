// Filename: config_egg_optchar.cxx
// Created by:  drose (18Jul03)
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

#include "config_egg_optchar.h"
#include "eggOptcharUserData.h"

#include "dconfig.h"


Configure(config_egg_optchar);

ConfigureFn(config_egg_optchar) {
  init_egg_optchar();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libegg_optchar
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_egg_optchar() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  EggOptcharUserData::init_type();
}
