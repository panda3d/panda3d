/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVEmulateTextureQueriesPass.cxx
 * @author rdb
 * @date 2024-11-19
 */

#include "spirVEmulateTextureQueriesPass.h"

/**
 *
 */
bool SpirVEmulateTextureQueriesPass::
transform_definition_op(Instruction op) {
  switch (op.opcode) {
  case spv::OpVariable:
    if (op.nargs >= 3) {
      uint32_t var_id = op.args[1];
      const Definition &var_def = _db.get_definition(var_id);
      if (var_def._flags & SpirVResultDatabase::DF_queried_image_size_levels) {
        _access_chains.insert({var_id, AccessChain(var_id)});
      }
      return true;
    }
    break;

  default:
    return SpirVTransformPass::transform_definition_op(op);
  }

  return true;
}

/**
 *
 */
bool SpirVEmulateTextureQueriesPass::
transform_function_op(Instruction op) {
  switch (op.opcode) {
  case spv::OpAccessChain:
  case spv::OpInBoundsAccessChain:
    if (op.nargs >= 4) {
      auto it = _access_chains.find(op.args[2]);
      if (it == _access_chains.end()) {
        return true;
      }
      AccessChain chain = it->second;
      uint32_t parent_id = unwrap_pointer_type(get_type_id(op.args[2]));

      for (size_t i = 3; i < op.nargs; ++i) {
        const Definition &index_def = _db.get_definition(op.args[i]);
        if (!index_def.is_constant()) {
          return true;
        }
        uint32_t index = index_def._constant;
        chain.append(index);

        const Definition &def = _db.get_definition(parent_id);
        if (def._members.empty()) { // array
          parent_id = def._type_id;
          nassertr(parent_id > 0, false);
        } else {
          // Must be a struct.
          parent_id = def._members[index]._type_id;
        }
      }

      _access_chains.insert({op.args[1], std::move(chain)});
    }
    break;

  case spv::OpLoad:
  case spv::OpCopyObject:
  case spv::OpCopyLogical:
  case spv::OpExpectKHR:
  case spv::OpImage:
  case spv::OpSampledImage:
    if (op.nargs >= 3) {
      auto it = _access_chains.find(op.args[2]);
      if (it != _access_chains.end()) {
        _access_chains.insert({op.args[1], it->second});
      }
    }
    break;

  case spv::OpImageQuerySize:
  case spv::OpImageQuerySizeLod:
  case spv::OpImageQueryLevels:
    if (op.nargs >= 3) {
      auto acit = _access_chains.find(op.args[2]);
      if (acit == _access_chains.end()) {
        return true;
      }
      const AccessChain &chain = acit->second;

      if (op.opcode == spv::OpImageQuerySizeLod && op.nargs >= 4) {
        const Definition &lod_def = _db.get_definition(op.args[3]);
        if (!lod_def.is_constant(0)) {
          // Can't handle a non-zero level of detail parameter.
          return true;
        }
      }

      uint32_t size_var_id;
      auto it = _size_var_ids.find(chain);
      if (it != _size_var_ids.end()) {
        size_var_id = it->second;
      } else {
        // It's always a vec4, with number of levels in fourth component
        const ShaderType *var_type = ShaderType::register_type(ShaderType::Vector(ShaderType::ST_float, 4));
        size_var_id = define_variable(var_type, spv::StorageClassUniformConstant);
        _size_var_ids.insert({std::move(chain), size_var_id});
      }

      uint32_t temp = op_load(size_var_id);

      // Grab the components we need out of the vec4 size variable.
      if (op.opcode == spv::OpImageQueryLevels) {
        temp = op_composite_extract(temp, {3});
      }
      else {
        const ShaderType *result_type = resolve_type(op.args[0]);
        if (result_type->as_scalar() != nullptr) {
          temp = op_composite_extract(temp, {0});
        }
        else if (const ShaderType::Vector *vector = result_type->as_vector()) {
          pvector<uint32_t> components;
          for (size_t i = 0; i < vector->get_num_components(); ++i) {
            components.push_back(i);
          }
          temp = op_vector_shuffle(temp, temp, std::move(components));
        }
        else {
          nassertr(false, true);
          return true;
        }
      }

      push_id(op.args[1]);
      op_convert(ShaderType::ST_int, temp);
      return false;
    }
    // fall through

  default:
    return SpirVTransformPass::transform_function_op(op);
  }

  return true;
}
