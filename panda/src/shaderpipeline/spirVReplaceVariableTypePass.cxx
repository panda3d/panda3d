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
      def._type_id = _pointer_type_id;
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
transform_function_op(Instruction op) {
  switch (op.opcode) {
  case spv::OpLoad:
    if (_pointer_ids.count(op.args[2])) {
      Definition &def = _db.modify_definition(op.args[1]);

      // If both are vectors or scalars, we can try a conversion.
      const ShaderType::Vector *old_vector = _new_type->as_vector();
      const ShaderType::Scalar *old_scalar = _new_type->as_scalar();
      const ShaderType::Vector *new_vector = def._type->as_vector();
      const ShaderType::Scalar *new_scalar = def._type->as_scalar();
      if ((old_vector != nullptr && new_vector != nullptr && old_vector != new_vector) ||
          (old_scalar != nullptr && new_scalar != nullptr && old_scalar != new_scalar) ||
          (old_vector != nullptr && new_scalar != nullptr) ||
          (old_scalar != nullptr && new_vector != nullptr)) {
        uint32_t temp = op_load(op.args[2]);
        ShaderType::ScalarType new_scalar_type;
        if (new_vector != nullptr && old_vector != nullptr) {
          // Swizzle the vector.
          new_scalar_type = new_vector->get_scalar_type();
          if (new_vector->get_num_components() != old_vector->get_num_components()) {
            pvector<uint32_t> components;
            uint32_t i = 0;
            while (i < new_vector->get_num_components() && i < old_vector->get_num_components()) {
              components.push_back(i);
              ++i;
            }
            // The remaining components are undefined.
            while (i < new_vector->get_num_components()) {
              components.push_back(0xffffffff);
              ++i;
            }
            temp = op_vector_shuffle(temp, temp, components);
          }
        }
        else if (new_vector != nullptr) {
          // Convert scalar to vector.
          new_scalar_type = new_vector->get_scalar_type();
          pvector<uint32_t> components(new_vector->get_num_components(), temp);
          temp = op_composite_construct(new_scalar, components);
        }
        else if (new_scalar != nullptr) {
          // Convert vector to scalar.
          new_scalar_type = new_scalar->get_scalar_type();
          temp = op_composite_extract(temp, {0});
        }
        else {
          new_scalar_type = new_scalar->get_scalar_type();
        }

        // Replace the original load with our conversion.
        push_id(op.args[1]);
        op_convert(new_scalar_type, temp);
        return false;
      }
      else {
        def._type = _new_type;
        def._type_id = _type_id;

        op.args[0] = _type_id;
        _object_ids.insert(op.args[1]);
      }
    }
    break;

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
    nassertd(_pointer_ids.count(op.args[3]) == _pointer_ids.count(op.args[4])) {}
    nassertd(_object_ids.count(op.args[3]) == _object_ids.count(op.args[4])) {}

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
    return SpirVTransformPass::transform_function_op(op);
  }

  return true;
}
