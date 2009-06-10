// Filename: config_cftalk.cxx
// Created by:  drose (26Mar09)
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

#include "config_cftalk.h"
#include "cfCommand.h"
#include "pandaSystem.h"

ConfigureDef(config_cftalk);
NotifyCategoryDef(cftalk, "");

ConfigureFn(config_cftalk) {
  init_libcftalk();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libcftalk
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libcftalk() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  CFCommand::init_type();
  CFDoCullCommand::init_type();

  CFDoCullCommand::register_with_read_factory();

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("cftalk");
}
