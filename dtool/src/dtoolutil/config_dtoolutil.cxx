/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_dtoolutil.cxx
 * @author drose
 * @date 2006-11-17
 */

#include "config_dtoolutil.h"

#include "filename.h"
#include "pandaSystem.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_DTOOL_DTOOLUTIL)
  #error Buildsystem error: BUILDING_DTOOL_DCTOOLUTIL not defined
#endif

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libdtoolutil() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  Filename::init_type();
  PandaSystem::init_type();
}

class InitDtoolutil {
public:
  InitDtoolutil() {
    init_libdtoolutil();
  }
};

static InitDtoolutil _init;
