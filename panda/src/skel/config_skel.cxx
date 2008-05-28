// Filename: config_skel.cxx
// Created by:  jyelon (09Feb07)
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

#include "config_skel.h"
#include "basicSkel.h"
#include "typedSkel.h"
#include "dconfig.h"

Configure(config_skel);
NotifyCategoryDef(skel, "");

ConfigureFn(config_skel) {
  init_libskel();
}

ConfigVariableInt skel_sample_config_variable
("skel-sample-config-variable", 3);

////////////////////////////////////////////////////////////////////
//     Function: init_libskel
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libskel() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  TypedSkel::init_type();
}

