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
#include "indent.h"

using std::string;

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
