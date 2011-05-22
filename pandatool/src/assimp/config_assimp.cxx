// Filename: config_assimp.cxx
// Created by:  rdb (29Mar11)
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

#include "config_assimp.h"

#include "loaderFileTypeAssimp.h"

#include "dconfig.h"
#include "loaderFileTypeRegistry.h"

ConfigureDef(config_assimp);
NotifyCategoryDef(assimp, "");

ConfigureFn(config_assimp) {
  init_libassimp();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libassimp
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libassimp() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  LoaderFileTypeAssimp::init_type();

  LoaderFileTypeRegistry *reg = LoaderFileTypeRegistry::get_global_ptr();
  reg->register_type(new LoaderFileTypeAssimp);
}
