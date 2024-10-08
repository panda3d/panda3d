/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVRemoveUnusedVariablesPass.cxx
 * @author rdb
 * @date 2024-10-08
 */

#include "spirVRemoveUnusedVariablesPass.h"

/**
 *
 */
void SpirVRemoveUnusedVariablesPass::
preprocess() {
  for (uint32_t id = 0; id < get_id_bound(); ++id) {
    Definition &def = _db.modify_definition(id);

    if (def.is_variable() && !def.is_used()) {
      if (shader_cat.is_debug() && !def._name.empty()) {
        shader_cat.debug()
          << "Removing unused variable " << def._name << " (" << id << ")\n";
      }
      def.clear();
      delete_id(id);
    }
  }

  // This is really all we need to do; the base class takes care of deletions.
}
