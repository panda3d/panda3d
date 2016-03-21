/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_ai.cxx
 * @author Pandai
 * @date 2009-09-13
 */

#include "config_ai.h"
#include "aiWorld.h"
#include "aiCharacter.h"
#include "aiBehaviors.h"
#include "seek.h"
#include "flee.h"
#include "pursue.h"
#include "evade.h"
#include "arrival.h"
#include "flock.h"
#include "wander.h"
#include "pathFollow.h"
#include "obstacleAvoidance.h"
#include "pathFind.h"
#include "aiNode.h"
#include "aiPathFinder.h"
#include "dconfig.h"

Configure(config_ai);
NotifyCategoryDef(ai, "");

ConfigureFn(config_ai) {
  init_libai();
}

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libai() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;
}
