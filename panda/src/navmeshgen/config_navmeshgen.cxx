/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_navmeshgen.cxx
 * @author ashwini
 * @date 2020-060-21
 */

#include "config_navmeshgen.h"

#include "pandaSystem.h"
#include "dconfig.h"
#include "navMeshBuilder.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_NAVMESHGEN)
  #error Buildsystem error: BUILDING_NAVMESHGEN not defined
#endif

Configure(config_navmeshgen);
NotifyCategoryDef(navmeshgen, "");

ConfigureFn(config_navmeshgen) {
  init_libnavmeshgen();
}


/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libnavmeshgen() {
  
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;
}

