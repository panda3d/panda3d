/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVReplaceVariableTypePass.cxx
 * @author rdb
 * @date 2024-10-08
 */

#include "spirVReplaceVariableTypePass.h"

/**
 *
 */
SpirVReplaceVariableTypePass::
SpirVReplaceVariableTypePass(uint32_t variable_id, const ShaderType *type,
                             spv::StorageClass storage_class) :
  _variable_id(variable_id),
  _new_type(type),
  _new_storage_class(storage_class) {

  _pointer_ids.insert(variable_id);
}

/**
 *
 */
bool SpirVReplaceVariableTypePass::
transform_definition_op(Instruction op) {
  if (op.opcode == spv::OpVariable) {
    if (op.args[1] == _variable_id) {
      Definition &def = _db.modify_definition(_variable_id);
      if (shader_cat.is_debug()) {
        shader_cat.debug()
          << "Changing type of variable " << _variable_id << " (" << def._name
          << ") from " << *def._type << " to " << *_new_type << "\n";
      }
      _pointer_type_id = define_pointer_type(_new_type, _new_storage_class);
      _type_id = unwrap_pointer_type(_pointer_type_id);
      add_definition(spv::OpVariable, {
        _pointer_type_id,
        _variable_id,
        (uint32_t)_new_storage_class,
      });
      def._type = _new_type;
      if (def.is_used()) {
        _db.mark_used(_variable_id);
      }
      return false;
    } else {
      return true;
    }
  } else {
    return SpirVTransformPass::transform_definition_op(op);
  }
}

/**
 *
 */
bool SpirVReplaceVariableTypePass::
transform_function_op(Instruction op, uint32_t function_id) {
  switch (op.opcode) {
  case spv::OpLoad:
  case spv::OpAtomicLoad:
  case spv::OpAtomicExchange:
  case spv::OpAtomicCompareExchange:
  case spv::OpAtomicCompareExchangeWeak:
  case spv::OpAtomicIIncrement:
  case spv::OpAtomicIDecrement:
  case spv::OpAtomicIAdd:
  case spv::OpAtomicISub:
  case spv::OpAtomicSMin:
  case spv::OpAtomicUMin:
  case spv::OpAtomicSMax:
  case spv::OpAtomicUMax:
  case spv::OpAtomicAnd:
  case spv::OpAtomicOr:
  case spv::OpAtomicXor:
  case spv::OpAtomicFMinEXT:
  case spv::OpAtomicFMaxEXT:
  case spv::OpAtomicFAddEXT:
    // These loads turn a pointer into a dereferenced object.
    if (_pointer_ids.count(op.args[2])) {
      Definition &def = _db.modify_definition(op.args[1]);
      op.args[0] = _type_id;
      def._type = _new_type;
      def._type_id = _type_id;
      _object_ids.insert(op.args[1]);
    }
    break;

  case spv::OpCopyObject:
  case spv::OpExpectKHR:
    // This clones a pointer or object verbatim, so keep following the chain.
    if (_pointer_ids.count(op.args[2])) {
      Definition &def = _db.modify_definition(op.args[1]);
      op.args[0] = _pointer_type_id;
      def._type = _new_type;
      def._type_id = _pointer_type_id;
      _pointer_ids.insert(op.args[1]);
    }
    if (_object_ids.count(op.args[2])) {
      Definition &def = _db.modify_definition(op.args[1]);
      op.args[0] = _type_id;
      def._type = _new_type;
      def._type_id = _type_id;
      _object_ids.insert(op.args[1]);
    }
    break;

  case spv::OpSelect:
    // The result type for this op must be the same for both operands.
    nassertd(_pointer_ids.count(op.args[3]) == _pointer_ids.count(op.args[4]));
    nassertd(_object_ids.count(op.args[3]) == _object_ids.count(op.args[4]));

    if (_pointer_ids.count(op.args[3])) {
      Definition &def = _db.modify_definition(op.args[1]);
      op.args[0] = _pointer_type_id;
      def._type = _new_type;
      def._type_id = _pointer_type_id;
      _pointer_ids.insert(op.args[1]);
    }
    if (_object_ids.count(op.args[3])) {
      Definition &def = _db.modify_definition(op.args[1]);
      op.args[0] = _type_id;
      def._type = _new_type;
      def._type_id = _type_id;
      _object_ids.insert(op.args[1]);
    }
    break;

  default:
    return SpirVTransformPass::transform_function_op(op, function_id);
  }

  return true;
}
