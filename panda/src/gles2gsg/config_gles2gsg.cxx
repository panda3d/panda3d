/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_gles2gsg.cxx
 * @author rdb
 * @date 2009-06-14
 */

#include "config_gles2gsg.h"
#include "gles2gsg.h"

#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDAGLES2)
  #error Buildsystem error: BUILDING_PANDAGLES2 not defined
#endif

ConfigureDef(config_gles2gsg);
NotifyCategoryDef(gles2gsg, ":display:gsg");

ConfigureFn(config_gles2gsg) {
  init_libgles2gsg();
}

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libgles2gsg() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  GLES2init_classes();
}
