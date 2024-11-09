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
#include "shaderCompilerGlslang.h"
#include "shaderCompilerGlslPreProc.h"

#include "shaderModuleGlsl.h"
#include "shaderModuleSpirV.h"

#include "dconfig.h"


ConfigureDef(config_shaderpipeline);

ConfigVariableBool glsl_preprocess
("glsl-preprocess", true,
 PRC_DESC("If this is enabled, Panda looks for lines starting with "
          "#pragma include when loading a GLSL shader and processes "
          "it appropriately.  This can be useful if you have code that "
          "is shared between multiple shaders.  Set this to false if "
          "you have no need for this feature or if you do your own "
          "preprocessing of GLSL shaders."));

ConfigVariableBool glsl_force_legacy_pipeline
("glsl-force-legacy-pipeline", false,
 PRC_DESC("If this is enabled, Panda will disable the new shader pipeline "
          "for all GLSL shaders, and all GLSL shaders will instead be "
          "compiled by the graphics driver.  If you enable this, GLSL shaders "
          "will only work if the graphics back-end supports that particular "
          "GLSL version, so this is not recommended.  Use this only if you "
          "suspect a bug in the new shader pipeline.  Note that GLSL shaders "
          "before version 150 always use the legacy GLSL pipeline, and Cg "
          "shaders always use the new pipeline."));

ConfigVariableInt glsl_include_recursion_limit
("glsl-include-recursion-limit", 10,
 PRC_DESC("This sets a limit on how many nested #pragma include "
          "directives that Panda will follow when glsl-preprocess is "
          "enabled.  This is used to prevent infinite recursion when "
          "two shader files include each other."));

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

  ShaderCompilerGlslang::init_type();
  ShaderModuleGlsl::init_type();
  ShaderModuleSpirV::init_type();

  ShaderModuleGlsl::register_with_read_factory();
  ShaderModuleSpirV::register_with_read_factory();

  ShaderCompilerRegistry *reg = ShaderCompilerRegistry::get_global_ptr();
  reg->register_compiler(new ShaderCompilerGlslang());
  reg->register_compiler(new ShaderCompilerGlslPreProc());
}
