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
#include "datagram.h"
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

/**
 * Returns a hash of every setting that can affect the output of the compiler.
 */
HashVal CompilerOptions::
get_cache_key() const {
  Datagram dg;
  dg.add_uint8((_flags & F_debug) != 0);
  dg.add_uint8((int)_optimize);

  size_t num_directories = _include_path.get_num_directories();
  dg.add_uint32((uint32_t)num_directories);
  for (size_t i = 0; i < num_directories; ++i) {
    dg.add_string(_include_path.get_directory(i));
  }

  dg.add_uint32((uint32_t)_defines.size());
  for (const auto &pair : _defines) {
    dg.add_string(pair.first);
    dg.add_string(pair.second);
  }

  HashVal hv;
  hv.hash_buffer((const char *)dg.get_data(), dg.get_length());
  return hv;
}
