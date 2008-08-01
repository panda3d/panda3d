// Filename: config_deadrec.cxx
// Created by:  drose (23Oct06)
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

#include "config_deadrec.h"

#include "dconfig.h"

Configure(config_deadrec);
NotifyCategoryDef(deadrec, "");

ConfigureFn(config_deadrec) {
  init_libdeadrec();
}

ConfigVariableBool accept_clock_skew
("accept-clock-skew", false,
 PRC_DESC("This controls the default value of "
          "SmoothMover::get_accept_clock_skew()."));


////////////////////////////////////////////////////////////////////
//     Function: init_libdeadrec
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libdeadrec() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;
}

