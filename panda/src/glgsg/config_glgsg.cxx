/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_glgsg.cxx
 * @author drose
 * @date 1999-10-06
 */

#include "config_glgsg.h"
#include "glgsg.h"

#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_GLGSG)
  #error Buildsystem error: BUILDING_PANDA_GLGSG not defined
#endif

ConfigureDef(config_glgsg);
NotifyCategoryDef(glgsg, ":display:gsg");

ConfigureFn(config_glgsg) {
  init_libglgsg();
}

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libglgsg() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  GLinit_classes();
}
