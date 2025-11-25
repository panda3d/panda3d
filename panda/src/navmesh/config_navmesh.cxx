/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_navmesh.cxx
 * @author Ashwani / 
 * @date 2024
 */

#include "config_navmesh.h"
#include "navMesh.h"
#include "navMeshBuilder.h"
#include "navMeshSettings.h"
#include "navPath.h"

#include "dconfig.h"

ConfigureDef(config_navmesh);
NotifyCategoryDef(navmesh, "");

ConfigureFn(config_navmesh) {
  init_libnavmesh();
}

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libnavmesh() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  NavMesh::init_type();
  NavMeshSettings::init_type();
  NavPath::init_type();
}

