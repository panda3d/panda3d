/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pandaode.cxx
 * @author drose
 * @date 2000-06-16
 */


#include "pandaode.h"
#include "config_ode.h"

// By including checkPandaVersion.h, we guarantee that runtime
// attempts to load libpandaode.so/.dll will fail if they
// inadvertently link with the wrong version of libdtool.so/.dll.

#include "checkPandaVersion.h"

/**
 * Initializes the library.  This must be called at least once before any of the
 * functions or classes in this library can be used.  Normally it will be called
 * by the static initializers and need not be called explicitly, but special
 * cases exist.
 */
void
init_libpandaode() {
  init_libode();
}
