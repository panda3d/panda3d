// Filename: config_effects.cxx
// Created by:  jason (18Jul00)
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

#include "config_effects.h"
#include "lensFlareNode.h"
#include "pandaSystem.h"
#include "dconfig.h"

Configure(config_effects);
NotifyCategoryDef(effects, "");

ConfigureFn(config_effects) {
  init_libeffects();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libeffects
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libeffects() {
#if 0  // temporarily disabled until we can port to new scene graph.
  LensFlareNode::init_type();
  LensFlareNode::register_with_read_factory();
#endif  // temporarily disabled until we can port to new scene graph.
}
