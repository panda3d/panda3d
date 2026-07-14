/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVInjectAlphaTestPass.cxx
 * @author rdb
 * @date 2024-11-10
 */

#include "spirVInjectAlphaTestPass.h"
#include "spirVInstructionCursor.h"

/**
 *
 */
void SpirVInjectAlphaTestPass::
run(SpirVModule &module) {
  if (_mode == M_always || _mode == M_none) {
    // You probably needn't have bothered running this pass...
    return;
  }

  spv::Op opcode = spv::OpNop;
  switch (_mode) {
  case M_never:
    break;
  case M_less:
    opcode = spv::OpFOrdGreaterThanEqual;
    break;
  case M_equal:
    opcode = spv::OpFUnordNotEqual;
    break;
  case M_less_equal:
    opcode = spv::OpFOrdGreaterThan;
    break;
  case M_greater:
    opcode = spv::OpFOrdLessThanEqual;
    break;
  case M_not_equal:
    opcode = spv::OpFOrdEqual;
    break;
  case M_greater_equal:
    opcode = spv::OpFOrdLessThan;
    break;
  default:
    nassertv(false);
    return;
  }

  // For each fragment entry point, find the output variable with location 0.
  pmap<Id, Id> entry_points;
  for (size_t i = 0; i < module.get_num_entry_points(); ++i) {
    const EntryPoint &ep = module.get_entry_point(i);
    if (ep.model == spv::ExecutionModelFragment) {
      for (Id var_id : ep.interface_vars) {
        if (module.get_location(var_id) == 0) {
          entry_points[ep.function_id] = var_id;
          break;
        }
      }
    }
  }

  for (const auto &item : entry_points) {
    Function *function = module.find_function(item.first);
    nassertd(function != nullptr) continue;
    Id var_id = item.second;

    // There may be multiple returns.  Insert an alpha test before every
    // return statement.
    SpirVInstructionCursor cursor(module, *function);
    while (cursor.next()) {
      spv::Op ret_opcode = cursor->opcode;
      if (ret_opcode != spv::OpReturn && ret_opcode != spv::OpReturnValue) {
        continue;
      }

      if (_mode == M_never) {
        // Replace the OpReturn with an OpKill.
        cursor.replace([](SpirVBuilder &builder) -> Id {
          builder.op_kill();
          return Id();
        });
        continue;
      }

      if (_alpha_ref_var_id == 0) {
        if (_spec_constant) {
          _alpha_ref_var_id = module.define_spec_constant(ShaderType::FLOAT, 0);
          if (_ref_location >= 0) {
            module.decorate(_alpha_ref_var_id, spv::DecorationSpecId, (uint32_t)_ref_location);
          }
        } else {
          _alpha_ref_var_id = module.define_variable(ShaderType::FLOAT, spv::StorageClassUniformConstant);
          if (_ref_location >= 0) {
            module.decorate(_alpha_ref_var_id, spv::DecorationLocation, (uint32_t)_ref_location);
          }
        }
      }

      cursor.insert_before([&](SpirVBuilder &builder) {
        Id alpha = builder.op_load(builder.op_access_chain(var_id, {module.define_int_constant(3)}));
        Id ref = _spec_constant ? _alpha_ref_var_id : builder.op_load(_alpha_ref_var_id);

        Id cond = builder.op_compare(opcode, alpha, ref);
        _compare_op_ids.push_back(cond);

        Id branch = builder.branch_if(cond);
          builder.op_kill();
        builder.branch_endif(branch);
      });
      // The cursor is back on the original return; next() continues past it.
    }
  }
}
