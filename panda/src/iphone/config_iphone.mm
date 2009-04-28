// Filename: config_iphone.cxx
// Created by:  drose (08Apr09)
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

#include "config_iphone.h"
#include "dconfig.h"

Configure(config_iphone);

NotifyCategoryDef(iphone, "");

ConfigureFn(config_iphone) {
  init_libiphone();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libiphone
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libiphone() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;
}
