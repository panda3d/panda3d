/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVConvertBoolToIntPass.cxx
 * @author rdb
 * @date 2024-10-14
 */

#include "spirVConvertBoolToIntPass.h"

/**
 *
 */
bool SpirVConvertBoolToIntPass::
transform_definition_op(Instruction op) {
  if (op.opcode == spv::OpVariable) {
    uint32_t var_id = op.args[1];
    spv::StorageClass storage_class = (spv::StorageClass)op.args[2];
    if (storage_class == spv::StorageClassUniformConstant) {
      const Definition &def = _db.get_definition(var_id);
      const ShaderType *new_type = def._type->replace_scalar_type(ShaderType::ST_bool, ShaderType::ST_int);
      if (def._type != new_type) {
        uint32_t pointer_type_id = define_pointer_type(new_type, (spv::StorageClass)op.args[2]);

        Definition &def = _db.modify_definition(var_id);
        def._type = new_type;
        def._type_id = pointer_type_id;
        op.args[0] = pointer_type_id;
        if (def.is_used()) {
          _db.mark_used(var_id);
        }
        _pointer_ids.insert(var_id);
      }
    }
    return true;
  } else {
    return SpirVTransformPass::transform_definition_op(op);
  }
}

/**
 *
 */
bool SpirVConvertBoolToIntPass::
transform_function_op(Instruction op) {
  switch (op.opcode) {
  case spv::OpAccessChain:
  case spv::OpInBoundsAccessChain:
    if (_pointer_ids.count(op.args[2])) {
      // Replace the access chain result type.
      Definition &pointer_type_def = _db.modify_definition(op.args[0]);
      const ShaderType *new_type = pointer_type_def._type->replace_scalar_type(ShaderType::ST_bool, ShaderType::ST_int);
      if (pointer_type_def._type != new_type) {
        Definition &result_def = _db.modify_definition(op.args[1]);
        result_def._type = new_type;
        result_def._type_id = define_pointer_type(new_type, pointer_type_def._storage_class);
        op.args[0] = result_def._type_id;
        _pointer_ids.insert(op.args[1]);
      }
    }
    break;

  case spv::OpLoad:
    if (_pointer_ids.count(op.args[2])) {
      // Load the int, then compare it with the null constant to produce the
      // desired bool.  Note that this may be an aggregate containing ints.
      uint32_t temp = op_load(op.args[2]);
      r_convert_to_bool(op.args[0], op.args[1], temp);
      return false;
    }
    break;

  case spv::OpStore:
    // Can't happen; we can't store to a UniformConstant.
    nassertr(!_pointer_ids.count(op.args[0]), false);
    /*if (_pointer_ids.count(op.args[0])) {
      // First convert to int.
      nassertr(resolve_type(get_type_id(op.args[1])) == ShaderType::bool_type, false);
      op.args[1] = op_select(op.args[1], define_int_constant(1), define_int_constant(0));
      op.args[0] = get_type_id(op.args[0]);
      return true;
    }*/
    break;

  case spv::OpCopyObject:
  case spv::OpExpectKHR:
    // This clones a pointer or object verbatim, so keep following the chain.
    if (_pointer_ids.count(op.args[2])) {
      Definition &def = _db.modify_definition(op.args[1]);
      const Definition &orig_def = _db.get_definition(op.args[2]);
      op.args[0] = orig_def._type_id;
      def._type = orig_def._type;
      def._type_id = orig_def._type_id;
      _pointer_ids.insert(op.args[1]);
    }
    break;

  default:
    return SpirVTransformPass::transform_function_op(op);
  }

  return true;
}

/**
 * Recursively convert the ints to bools in the given object.
 */
void SpirVConvertBoolToIntPass::
r_convert_to_bool(uint32_t to_type_id, uint32_t to_id, uint32_t from_id) {
  const ShaderType *to_type = resolve_type(to_type_id);

  if (const ShaderType::Array *array_type = to_type->as_array()) {
    const ShaderType *element_type = array_type->get_element_type();
    uint32_t element_type_id = define_type(element_type);

    pvector<uint32_t> args({to_type_id, to_id});
    for (uint32_t i = 0; i < array_type->get_num_elements(); ++i) {
      uint32_t temp = op_composite_extract(from_id, {i});
      uint32_t into = allocate_id();
      r_convert_to_bool(element_type_id, into, temp);
      args.push_back(into);
    }

    add_instruction(spv::OpCompositeConstruct, args.data(), args.size());
  }
  else if (const ShaderType::Struct *struct_type = to_type->as_struct()) {
    pvector<uint32_t> args({to_type_id, to_id});

    for (uint32_t i = 0; i < struct_type->get_num_members(); ++i) {
      const ShaderType::Struct::Member &member = struct_type->get_member(i);

      uint32_t temp = op_composite_extract(from_id, {i});

      if (member.type->contains_scalar_type(ShaderType::ST_bool)) {
        uint32_t into = allocate_id();
        r_convert_to_bool(define_type(member.type), into, temp);
        temp = into;
      }

      args.push_back(temp);
    }

    add_instruction(spv::OpCompositeConstruct, args.data(), args.size());
  }
  else {
    const ShaderType *from_type = resolve_type(get_type_id(from_id));
    uint32_t null = define_null_constant(from_type);
    add_instruction(spv::OpINotEqual, {to_type_id, to_id, from_id, null});
  }
}
