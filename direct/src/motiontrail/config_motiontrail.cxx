/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_motiontrail.cxx
 * @author drose
 * @date 2002-08-27
 */

#include "config_motiontrail.h"
#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_DIRECT_MOTIONTRAIL)
  #error Buildsystem error: BUILDING_DIRECT_MOTIONTRAIL not defined
#endif

extern EXPCL_DIRECT_MOTIONTRAIL void init_libmotiontrail();

Configure(config_motiontrail);
NotifyCategoryDef(motiontrail, "");

ConfigureFn(config_motiontrail) {
  init_libmotiontrail();
}

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libmotiontrail() {
  static bool initialized = false;
  if (initialized == false) {
    CMotionTrail::init_type();
    initialized = true;
  }
}
