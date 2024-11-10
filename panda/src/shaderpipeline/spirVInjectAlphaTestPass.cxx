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

/**
 * Transforms an OpEntryPoint.
 * Return true to keep the instruction, false to omit it.
 */
bool SpirVInjectAlphaTestPass::
transform_entry_point(spv::ExecutionModel model, uint32_t id, const char *name, const uint32_t *var_ids, uint16_t num_vars) {
  if (model == spv::ExecutionModelFragment) {
    for (size_t i = 0; i < num_vars; ++i) {
      uint32_t var_id = var_ids[i];
      if (_db.get_definition(var_id)._location == 0) {
        _entry_points[id] = var_id;
        break;
      }
    }
  }
  return true;
}

/**
 *
 */
bool SpirVInjectAlphaTestPass::
begin_function(Instruction op) {
  auto it = _entry_points.find(op.args[1]);
  if (it != _entry_points.end()) {
    _var_id = it->second;
  } else {
    _var_id = 0;
  }
  return true;
}

/**
 *
 */
bool SpirVInjectAlphaTestPass::
transform_function_op(Instruction op) {
  // There may be multiple returns.  Insert an alpha test before every return
  // statement.
  if (_var_id != 0 &&
      (op.opcode == spv::OpReturn || op.opcode == spv::OpReturnValue)) {

    spv::Op opcode;
    switch (_mode) {
    case M_always:
    case M_none:
      // You probably needn't have bothered running this pass...
      return true;

    case M_never:
      // Replace the OpReturn with an OpKill.
      op_kill();
      return false;

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
      nassertr(false, true);
      return true;
    }

    if (_alpha_ref_var_id == 0) {
      _alpha_ref_var_id = define_variable(ShaderType::float_type, spv::StorageClassUniformConstant);
      if (_ref_location >= 0) {
        decorate(_alpha_ref_var_id, spv::DecorationLocation, (uint32_t)_ref_location);
      }
    }
    uint32_t alpha = op_load(op_access_chain(_var_id, {define_int_constant(3)}));
    uint32_t ref = op_load(_alpha_ref_var_id);

    uint32_t branch = branch_if(op_compare(opcode, alpha, ref));
      op_kill();
    branch_endif(branch);
  }

  return SpirVTransformPass::transform_function_op(op);
}

/**
 * Called when an OpFunctionEnd instruction is encountered, belonging to an
 * OpFunction with the given identifier.
 */
void SpirVInjectAlphaTestPass::
end_function(uint32_t function_id) {
  _var_id = 0;
}
