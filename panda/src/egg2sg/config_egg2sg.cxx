// Filename: config_egg2sg.cxx
// Created by:  drose (01Oct99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "config_egg2sg.h"
#include "loaderFileTypeEgg.h"

#include <dconfig.h>
#include <loaderFileTypeRegistry.h>
#include <get_config_path.h>

ConfigureDef(config_egg2sg);
NotifyCategoryDef(egg2sg, "");

ConfigureFn(config_egg2sg) {
  init_libegg2sg();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libegg2sg
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libegg2sg() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  LoaderFileTypeEgg::init_type();

  LoaderFileTypeRegistry *reg = LoaderFileTypeRegistry::get_ptr();

  reg->register_type(new LoaderFileTypeEgg);
}
