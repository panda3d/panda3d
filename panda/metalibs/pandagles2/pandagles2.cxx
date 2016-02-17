/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pandagles2.cxx
 * @author rdb
 * @date 2009-06-08
 */

#include "pandagles2.h"

#define OPENGLES_2
#include "config_gles2gsg.h"

#include "config_egldisplay.h"
#include "eglGraphicsPipe.h"

// By including checkPandaVersion.h, we guarantee that runtime
// attempts to load libpandagles2.so/.dll will fail if they inadvertently
// link with the wrong version of libdtool.so/.dll.

#include "checkPandaVersion.h"

/**
 * Initializes the library.  This must be called at least once before any of the
 * functions or classes in this library can be used.  Normally it will be called
 * by the static initializers and need not be called explicitly, but special
 * cases exist.
 */
void
init_libpandagles2() {
  init_libgles2gsg();
  init_libegldisplay();
}

/**
 * Returns the TypeHandle index of the recommended graphics pipe type defined by
 * this module.
 */
int
get_pipe_type_pandagles2() {
  return eglGraphicsPipe::get_class_type().get_index();
}
