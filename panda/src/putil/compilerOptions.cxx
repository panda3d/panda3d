/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file compilerOptions.cxx
 * @author rdb
 * @date 2025-12-23
 */

#include "compilerOptions.h"
#include "config_putil.h"
#include "configVariableBool.h"
#include "indent.h"

using std::string;

static ConfigVariableBool shader_debug
("shader-debug", false,
 PRC_DESC("If this is enabled, shaders will be compiled with debugging "
          "support.  This preserves source information (line numbers and "
          "variable names) in the compiled shader module, and enables "
          "features such as debugPrintfEXT and shader assertions.  "
          "Enabling this may increase memory usage and reduce performance. "
          "Individual shaders can also opt in or out via CompilerOptions.debug."));

/**
 *
 */
CompilerOptions::
CompilerOptions() : _include_path(get_model_path()) {
  if (shader_debug) {
    _flags |= F_debug;
  }
}

/**
 *
 */
void CompilerOptions::
output(std::ostream &out) const {
  out << "CompilerOptions(";
  out << ")";
}

/**
 * Writes the definitions to the given output stream.
 */
void CompilerOptions::
write_defines(std::ostream &out) const {
  for (const auto &pair : _defines) {
    if (pair.second.empty()) {
      out << "#define " << pair.first << "\n";
    } else {
      out << "#define " << pair.first << " " << pair.second << "\n";
    }
  }
}
