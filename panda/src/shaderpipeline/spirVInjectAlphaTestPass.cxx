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

  // The comparison under which the fragment is *discarded*: the negation of
  // the mode under which it passes.  != is unordered so that a NaN alpha is
  // discarded by M_equal, matching the FOrd comparisons discarding it in
  // every other mode.
  Value (*discard_if)(Value, glsl::Argument) = nullptr;
  switch (_mode) {
  case M_never:
    break;
  case M_less:
    discard_if = &glsl::greater_than_equal;
    break;
  case M_equal:
    discard_if = &glsl::not_equal;
    break;
  case M_less_equal:
    discard_if = &glsl::greater_than;
    break;
  case M_greater:
    discard_if = &glsl::less_than_equal;
    break;
  case M_not_equal:
    discard_if = &glsl::equal;
    break;
  case M_greater_equal:
    discard_if = &glsl::less_than;
    break;
  default:
    nassertv(false);
    return;
  }

  // Find the color output: the module-scope output variable with location 0.
  Id var_id;
  for (uint32_t word = 0; word < module.get_id_bound(); ++word) {
    Id id(word);
    if (module.get_definition_type(id) == SpirVModule::DT_variable &&
        module.get_function_id(id) == 0 &&
        module.get_storage_class(id) == spv::StorageClassOutput &&
        module.get_location(id) == 0) {
      var_id = id;
      break;
    }
  }

  // If the output is not a vector with an alpha channel, such as a vec3 or
  // a missing output, the implicit alpha value is 1.
  bool has_alpha = false;
  if (var_id != 0) {
    const ShaderType *type = module.resolve_type(var_id);
    const ShaderType::Vector *vector = (type != nullptr) ? type->as_vector() : nullptr;
    has_alpha = vector != nullptr &&
                vector->get_scalar_type() == ShaderType::ST_float &&
                vector->get_num_components() == 4;
  }

  for (size_t i = 0; i < module.get_num_entry_points(); ++i) {
    const EntryPoint &ep = module.get_entry_point(i);
    if (ep.model != spv::ExecutionModelFragment) {
      continue;
    }

    Function *function = module.find_function(ep.function_id);
    nassertd(function != nullptr) continue;

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
        Value alpha;
        if (has_alpha) {
          alpha = Pointer(builder, var_id)[3].load();
        } else {
          alpha = builder.const_float(1.0f);
        }
        Value ref = _spec_constant ? Value(builder, _alpha_ref_var_id)
                                   : Pointer(builder, _alpha_ref_var_id).load();

        Value cond = discard_if(alpha, ref);
        _compare_op_ids.push_back(cond);

        builder.kill_if(cond);
      });
      // The cursor is back on the original return; next() continues past it.
    }
  }
}
