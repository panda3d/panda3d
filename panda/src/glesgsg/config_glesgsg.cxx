/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_glesgsg.cxx
 * @author rdb
 * @date 2009-05-21
 */

#include "config_glesgsg.h"
#include "glesgsg.h"

#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDAGLES)
  #error Buildsystem error: BUILDING_PANDAGLES not defined
#endif

ConfigureDef(config_glesgsg);
NotifyCategoryDef(glesgsg, ":display:gsg");

ConfigureFn(config_glesgsg) {
  init_libglesgsg();
}

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libglesgsg() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  GLESinit_classes();
}
