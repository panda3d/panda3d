/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_shaderpipeline.cxx
 * @author Mitchell Stokes
 * @date 2019-02-16
 */

#include "config_shaderpipeline.h"

#include "shaderCompilerRegistry.h"
#include "shaderCompilerGlslPreProc.h"

#include "dconfig.h"


ConfigureDef(config_shaderpipeline);
NotifyCategoryDef(shaderpipeline, "");

ConfigureFn(config_shaderpipeline) {
  init_libshaderpipeline();
}

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libshaderpipeline() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  ShaderCompilerRegistry *reg = ShaderCompilerRegistry::get_global_ptr();
  reg->register_compiler(new ShaderCompilerGlslPreProc());
}
