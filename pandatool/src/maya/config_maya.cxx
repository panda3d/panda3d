// Filename: config_maya.cxx
// Created by:  drose (15Apr02)
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

#include "config_maya.h"

#include "dconfig.h"

Configure(config_maya);
NotifyCategoryDef(maya, "");

ConfigureFn(config_maya) {
  init_libmaya();
}

ConfigVariableInt init_maya_repeat_count
("init-maya-repeat-count", 5,
 PRC_DESC("The number of times to attempt to initialize Maya and acquire the "
          "Maya license before giving up."));

ConfigVariableDouble init_maya_timeout
("init-maya-timeout", 5.0,
 PRC_DESC("The number of seconds to wait between attempts to acquire the "
          "Maya license."));

////////////////////////////////////////////////////////////////////
//     Function: init_libmaya
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libmaya() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;
}

