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
#include "spirVUsageAnalysis.h"

/**
 *
 */
void SpirVRemoveUnusedVariablesPass::
run(SpirVModule &module) {
  SpirVUsageAnalysis usage = module.analyze_usage();

  pmap<Id, BitArray> ftype_params_used;
  pset<Id> dead_ids;

  for (uint32_t word = 0; word < module.get_id_bound(); ++word) {
    Id id(word);
    if (module.get_definition_type(id) == SpirVModule::DT_variable && !usage.is_used(id)) {
      if (shader_cat.is_debug()) {
        shader_cat.debug()
          << "Removing unused variable " << module.get_name(id)
          << " (" << id << ")\n";
      }
      module.delete_id(id);
      dead_ids.insert(id);
    }
  }

  for (size_t i = 0; i < module.get_num_functions(); ++i) {
    const Function &function = module.get_function(i);
    const pvector<Id> &params = function.parameters;
    for (size_t i = 0; i < params.size(); ++i) {
      // Only unused pointer parameters can be removed; a value parameter may
      // not simply have its argument dropped without changing semantics.
      if (usage.is_used(params[i]) ||
          module.get_definition_type(module.get_type_id(params[i])) != SpirVModule::DT_pointer_type) {
        ftype_params_used[function.type_id].set_bit(i);
      }
    }
  }

  // Remove any instructions based on the deleted variables: access chains
  // and pointer copies (which never mark their variable used), and loads of
  // opaque values that nothing consumed (which is what left the variable
  // unused in the first place).
  module.delete_dead_code(dead_ids);

  for (const auto &item : ftype_params_used) {
    Id func_type_id = item.first;
    uint32_t num_params = 0;
    {
      const Instruction *decl = module.find_declaration(func_type_id);
      nassertd(decl != nullptr && decl->opcode == spv::OpTypeFunction) continue;
      num_params = (uint32_t)decl->args.size() - 2;
    }

    pset<uint32_t> deleted_params;
    for (uint32_t i = 0; i < num_params; ++i) {
      if (!item.second.get_bit(i)) {
        if (shader_cat.is_debug()) {
          shader_cat.debug()
            << "Removing unused function parameter " << i
            << " from function type " << func_type_id << "\n";
        }
        deleted_params.insert(i);
      }
    }
    module.remove_function_parameters(func_type_id, deleted_params);
  }
}
