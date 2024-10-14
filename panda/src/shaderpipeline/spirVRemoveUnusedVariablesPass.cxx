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
  pmap<uint32_t, BitArray> ftype_params_used;

  for (uint32_t id = 0; id < get_id_bound(); ++id) {
    Definition &def = _db.modify_definition(id);

    if (def.is_variable() && !def.is_used()) {
      if (shader_cat.is_debug()) {
        shader_cat.debug()
          << "Removing unused variable " << def._name << " (" << id << ")\n";
      }
      def.clear();
      delete_id(id);
    }
    if (def.is_function()) {
      for (size_t i = 0; i < def._parameters.size(); ++i) {
        uint32_t param_id = def._parameters[i];
        const Definition &param_def = _db.get_definition(param_id);
        if (param_def.is_used() || !_db.get_definition(param_def._type_id).is_pointer_type()) {
          ftype_params_used[def._type_id].set_bit(i);
        }
      }
    }
  }

  for (auto it = ftype_params_used.begin(); it != ftype_params_used.end(); ++it) {
    uint32_t func_type_id = it->first;
    const Definition &def = _db.get_definition(func_type_id);

    for (size_t i = 0; i < def._parameters.size(); ++i) {
      if (!it->second.get_bit(i)) {
        if (shader_cat.is_debug()) {
          shader_cat.debug()
            << "Removing unused function parameter " << i
            << " from function type " << func_type_id << "\n";
        }
        delete_function_parameter(func_type_id, i);
      }
    }
  }

  // This is really all we need to do; the base class takes care of deletions.
}
