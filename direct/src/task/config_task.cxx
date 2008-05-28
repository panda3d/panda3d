// Filename: config_task.cxx
// Created by:  Administrator (03Sep04)
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


#include "config_task.h"
#include "cTask.h"

#include "dconfig.h"

Configure(config_task);
NotifyCategoryDef(task, "");

ConfigureFn(config_task) {
  init_libtask();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libtask
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libtask() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  CTask::init_type();
}
