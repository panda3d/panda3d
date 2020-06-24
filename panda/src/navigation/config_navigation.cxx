/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_navigation.cxx
 * @author ashwini
 * @date 2020-060-21
 */

#include "config_navigation.h"

#include "pandaSystem.h"
#include "dconfig.h"
#include "navMesh.h"
#include "navMeshBuilder.h"
#include "navMeshNode.h"
#include "navMeshQuery.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_RECASTDETOUR)
  #error Buildsystem error: BUILDING_NAVIGATION not defined
#endif

Configure(config_navigation);
NotifyCategoryDef(navigation, "");

ConfigureFn(config_navigation) {
  init_libnavigation();
}

ConfigVariableInt navigation_sample_config_variable
("navigation-sample-config-variable", 25);

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libnavigation() {
  NavMesh::init_type();
  NavMeshNode::init_type();
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;
}

