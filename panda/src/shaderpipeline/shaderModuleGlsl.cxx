/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderModuleGlsl.cxx
 * @author Mitchell Stokes
 * @date 2019-02-16
 */

#include "shaderModuleGlsl.h"
#include "string_utils.h"

TypeHandle ShaderModuleGlsl::_type_handle;

ShaderModuleGlsl::
ShaderModuleGlsl(Stage stage) :
  ShaderModule(stage)
{
}

ShaderModuleGlsl::
~ShaderModuleGlsl() {
}

std::string ShaderModuleGlsl::
get_ir() const {
  return this->_raw_source;
}

/**
 * Returns the filename of the included shader with the given source file
 * index (as recorded in the #line statement in r_preprocess_source).  We use
 * this to associate error messages with included files.
 */
Filename ShaderModuleGlsl::
get_filename_from_index(int index) const {
  if (index == 0) {
    Filename fn = get_source_filename();
    if (!fn.empty()) {
      return fn;
    }
  } else if (glsl_preprocess && index >= 2048 &&
             (index - 2048) < (int)_included_files.size()) {
    return _included_files[(size_t)index - 2048];
  }

  // Must be a mistake.  Quietly put back the integer.
  std::string str = format_string(index);
  return Filename(str);
}
