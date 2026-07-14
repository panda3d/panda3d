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
#include "spirVInstructionCursor.h"

/**
 *
 */
SpirVReplaceVariableTypePass::
SpirVReplaceVariableTypePass(Id variable_id, const ShaderType *type,
                             spv::StorageClass storage_class) :
  _variable_id(variable_id),
  _new_type(type),
  _new_storage_class(storage_class) {

  _pointer_ids.insert(variable_id);
}

/**
 *
 */
void SpirVReplaceVariableTypePass::
run(SpirVModule &module) {
  // Retype the variable declaration itself.
  {
    if (shader_cat.is_debug()) {
      const ShaderType *old_type = module.resolve_type(_variable_id);
      shader_cat.debug()
        << "Changing type of variable " << _variable_id << " ("
        << module.get_name(_variable_id) << ") from ";
      if (old_type != nullptr) {
        shader_cat.debug(false) << *old_type;
      } else {
        shader_cat.debug(false) << "(unknown)";
      }
      shader_cat.debug(false) << " to " << *_new_type << "\n";
    }

    _pointer_type_id = module.define_pointer_type(_new_type, _new_storage_class);
    _type_id = module.unwrap_pointer_type(_pointer_type_id);

    // The new pointer and pointee types are now available; update the variable
    // declaration to refer to them.
    Instruction *decl = module.find_declaration(_variable_id);
    nassertv(decl != nullptr && decl->opcode == spv::OpVariable);

    decl->args[0] = _pointer_type_id;
    decl->args[2] = (uint32_t)_new_storage_class;
    module.record_result(*decl, Id());
  }

  // Now fix up all the loads and copies of the variable.
  for (Function &function : module.modify_functions()) {
    SpirVInstructionCursor cursor(module, function);
    while (cursor.next()) {
      switch (cursor->opcode) {
      case spv::OpLoad:
        if (_pointer_ids.count(Id(cursor->args[2]))) {
          Id result_id(cursor->args[1]);
          Id var_id(cursor->args[2]);

          // The type the rest of the shader expects to load.
          const ShaderType *result_type = module.resolve_type(result_id);
          nassertv(result_type != nullptr);

          // If both are vectors or scalars, we can try a conversion.
          const ShaderType::Vector *old_vector = _new_type->as_vector();
          const ShaderType::Scalar *old_scalar = _new_type->as_scalar();
          const ShaderType::Vector *new_vector = result_type->as_vector();
          const ShaderType::Scalar *new_scalar = result_type->as_scalar();
          if ((old_vector != nullptr && new_vector != nullptr && old_vector != new_vector) ||
              (old_scalar != nullptr && new_scalar != nullptr && old_scalar != new_scalar) ||
              (old_vector != nullptr && new_scalar != nullptr) ||
              (old_scalar != nullptr && new_vector != nullptr)) {
            // Replace the original load with a load of the new type followed
            // by a conversion, of which the final op takes over the original
            // result id.
            cursor.replace([&](SpirVBuilder &builder) -> Id {
              Id temp = builder.op_load(var_id);

              if (new_vector != nullptr && old_vector != nullptr) {
                // Swizzle the vector.
                if (new_vector->get_num_components() != old_vector->get_num_components()) {
                  pvector<uint32_t> components;
                  uint32_t ci = 0;
                  while (ci < new_vector->get_num_components() && ci < old_vector->get_num_components()) {
                    components.push_back(ci);
                    ++ci;
                  }
                  // The remaining components are undefined.
                  while (ci < new_vector->get_num_components()) {
                    components.push_back(0xffffffff);
                    ++ci;
                  }
                  temp = builder.op_vector_shuffle(temp, temp, components);
                }
                return builder.op_convert(new_vector->get_scalar_type(), temp);
              }
              if (new_vector != nullptr) {
                // Convert the scalar, then replicate it into a vector.
                temp = builder.op_convert(new_vector->get_scalar_type(), temp);
                pvector<Id> components(new_vector->get_num_components(), temp);
                return builder.op_composite_construct(new_vector, components);
              }
              if (old_vector != nullptr) {
                // Take the first component of the vector.
                temp = builder.op_composite_extract(temp, {0});
              }
              return builder.op_convert(new_scalar->get_scalar_type(), temp);
            });
          }
          else {
            cursor->args[0] = _type_id;
            module.record_result(*cursor, function.id);
            _object_ids.insert(result_id);
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
        if (_pointer_ids.count(Id(cursor->args[2]))) {
          Instruction &op = *cursor;
          op.args[0] = _type_id;
          module.record_result(op, function.id);
          _object_ids.insert(Id(op.args[1]));
        }
        break;

      case spv::OpCopyObject:
      case spv::OpExpectKHR:
        // This clones a pointer or object verbatim, so keep following the chain.
        {
          Instruction &op = *cursor;
          if (_pointer_ids.count(Id(op.args[2]))) {
            op.args[0] = _pointer_type_id;
            module.record_result(op, function.id);
            _pointer_ids.insert(Id(op.args[1]));
          }
          if (_object_ids.count(Id(op.args[2]))) {
            op.args[0] = _type_id;
            module.record_result(op, function.id);
            _object_ids.insert(Id(op.args[1]));
          }
        }
        break;

      case spv::OpSelect:
        {
          Instruction &op = *cursor;

          // The result type for this op must be the same for both operands.
          nassertd(_pointer_ids.count(Id(op.args[3])) == _pointer_ids.count(Id(op.args[4]))) {}
          nassertd(_object_ids.count(Id(op.args[3])) == _object_ids.count(Id(op.args[4]))) {}

          if (_pointer_ids.count(Id(op.args[3]))) {
            op.args[0] = _pointer_type_id;
            module.record_result(op, function.id);
            _pointer_ids.insert(Id(op.args[1]));
          }
          if (_object_ids.count(Id(op.args[3]))) {
            op.args[0] = _type_id;
            module.record_result(op, function.id);
            _object_ids.insert(Id(op.args[1]));
          }
        }
        break;

      default:
        break;
      }
    }
  }
}
