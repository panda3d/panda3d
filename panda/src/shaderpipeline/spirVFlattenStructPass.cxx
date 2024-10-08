/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVFlattenStructPass.cxx
 * @author rdb
 * @date 2024-10-08
 */

#include "spirVFlattenStructPass.h"

/**
 *
 */
SpirVFlattenStructPass::
SpirVFlattenStructPass(uint32_t type_id) : _type_id(type_id) {
}

/**
 *
 */
void SpirVFlattenStructPass::
preprocess() {
  const Definition &def = _db.get_definition(_type_id);
  DCAST_INTO_V(_struct_type, def._type);

  _member_ids.resize(def._members.size(), 0);

  // Mark this as deleted up front, so that all annotations etc. will be
  // removed by the base class
  delete_id(_type_id);
}

/**
 *
 */
bool SpirVFlattenStructPass::
transform_definition_op(Instruction op) {
  switch (op.opcode) {
  case spv::OpVariable:
    if (op.nargs >= 3 && is_deleted(op.args[0])) {
      // Delete this variable entirely, and replace it instead with individual
      // variable definitions for all its members.
      uint32_t struct_var_id = op.args[1];
      Definition &struct_def = _db.modify_definition(struct_var_id);
      int struct_location = struct_def._location;

      std::string struct_var_name = std::move(struct_def._name);
      if (shader_cat.is_spam()) {
        shader_cat.spam()
          << "Removing variable " << struct_var_id << ": "
          << *struct_def._type << " " << struct_var_name << "\n";
      }
      struct_def.clear();
      delete_id(struct_var_id);

      for (size_t mi = 0; mi < _struct_type->get_num_members(); ++mi) {
        const ShaderType::Struct::Member &member = _struct_type->get_member(mi);

        // Insert a new variable for this struct member.
        uint32_t variable_id = define_variable(member.type, spv::StorageClassUniformConstant);
        if (!member.name.empty()) {
          add_name(variable_id, member.name);
        }

        Definition &variable_def = _db.modify_definition(variable_id);
        if (struct_var_name.empty()) {
          variable_def._name = member.name;
        } else {
          variable_def._name = struct_var_name + "." + member.name;
        }
        if (struct_location >= 0) {
          // Assign decorations to the individual members.
          int location = struct_location + mi;
          variable_def._location = location;

          add_annotation(spv::OpDecorate,
            {variable_id, spv::DecorationLocation, (uint32_t)location});
        }

        _member_ids[mi] = variable_id;
      }
      return false;
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
bool SpirVFlattenStructPass::
transform_function_op(Instruction op, uint32_t function_id) {
  switch (op.opcode) {
  case spv::OpAccessChain:
  case spv::OpInBoundsAccessChain:
  case spv::OpPtrAccessChain:
  case spv::OpInBoundsPtrAccessChain:
    if (_deleted_access_chains.count(op.args[2])) {
      // The base of this access chain is an access chain we deleted.
      uint32_t new_var_id = _deleted_access_chains[op.args[2]];

      const ShaderType *type = resolve_pointer_type(op.args[0]);
      uint32_t pointer_type_id = define_pointer_type(type, spv::StorageClassUniformConstant);

      pvector<uint32_t> new_args({pointer_type_id, op.args[1], new_var_id});
      new_args.insert(new_args.end(), op.args + 3, op.args + op.nargs);
      add_instruction(op.opcode, new_args.data(), new_args.size());

      Definition &def = _db.modify_definition(op.args[1]);
      def._type_id = pointer_type_id;
      def._origin_id = new_var_id;
      return false;
    }
    else if (is_deleted(op.args[2])) {
      uint32_t index = resolve_constant(op.args[3]);
      if (op.nargs > 4) {
        // We also need to change the type if it has the wrong storage class.
        const ShaderType *type = resolve_pointer_type(op.args[0]);
        uint32_t pointer_type_id = define_pointer_type(type, spv::StorageClassUniformConstant);

        // Just unwrap the first index.
        pvector<uint32_t> new_args({pointer_type_id, op.args[1], _member_ids[index]});
        new_args.insert(new_args.end(), op.args + 4, op.args + op.nargs);
        add_instruction(op.opcode, new_args.data(), new_args.size());

        // Change the origin so that future loads through this access chain
        // will be able to mark the new variable as used.
        Definition &def = _db.modify_definition(op.args[1]);
        def._type_id = pointer_type_id;
        def._origin_id = _member_ids[index];
      } else {
        // Delete the access chain entirely.
        _deleted_access_chains[op.args[1]] = _member_ids[index];
        delete_id(op.args[1]);
      }
      return false;
    }
    break;

  case spv::OpFunctionCall:
    for (size_t i = 3; i < op.nargs; ++i) {
      if (_deleted_access_chains.count(op.args[i])) {
        op.args[i] = _deleted_access_chains[op.args[i]];
      }
      mark_used(op.args[i]);
    }
    break;

  case spv::OpImageTexelPointer:
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
  case spv::OpAtomicFlagTestAndSet:
  case spv::OpAtomicFMinEXT:
  case spv::OpAtomicFMaxEXT:
  case spv::OpAtomicFAddEXT:
    // If this triggers, the struct is being loaded into another variable,
    // which means we can't unwrap this (for now).
    {
      Definition &def = _db.modify_definition(op.args[1]);
      if (_deleted_access_chains.count(op.args[2])) {
        op.args[2] = _deleted_access_chains[op.args[2]];
        def._origin_id = op.args[2];
      }
      else if (is_deleted(def._origin_id)) {
        // Origin points to deleted variable, change to proper variable.
        const Definition &from = _db.get_definition(op.args[2]);
        def._origin_id = from._origin_id;
      }
    }
    nassertr(!is_deleted(op.args[2]), true);
    mark_used(op.args[1]);
    break;

  case spv::OpStore:
  case spv::OpAtomicStore:
  case spv::OpAtomicFlagClear:
    // Can't store the struct pointer itself (yet)
    if (_deleted_access_chains.count(op.args[0])) {
      op.args[0] = _deleted_access_chains[op.args[0]];
    }
    nassertr(!is_deleted(op.args[0]), true);
    mark_used(op.args[0]);
    break;

  case spv::OpCopyMemory:
  case spv::OpCopyMemorySized:
    // Shouldn't be copying into or out of the struct directly.
    if (_deleted_access_chains.count(op.args[0])) {
      op.args[0] = _deleted_access_chains[op.args[0]];
    }
    nassertr(!is_deleted(op.args[0]), true);
    if (_deleted_access_chains.count(op.args[1])) {
      op.args[1] = _deleted_access_chains[op.args[1]];
    }
    nassertr(!is_deleted(op.args[1]), true);
    mark_used(op.args[0]);
    mark_used(op.args[1]);
    break;

  case spv::OpArrayLength:
  case spv::OpConvertPtrToU:
    if (_deleted_access_chains.count(op.args[2])) {
      op.args[2] = _deleted_access_chains[op.args[2]];
    }
    nassertr(!is_deleted(op.args[2]), true);
    mark_used(op.args[2]);
    break;

  case spv::OpCopyObject:
    if (_deleted_access_chains.count(op.args[2])) {
      op.args[2] = _deleted_access_chains[op.args[2]];

      Definition &def = _db.modify_definition(op.args[1]);
      def._origin_id = op.args[2];
      def._type_id = get_type_id(op.args[2]);

      // Copy the type since the storage class may have changed.
      op.args[0] = def._type_id;
    }
    else if (is_deleted(op.args[2])) {
      // If it's just a copy of the struct pointer, delete the copy.
      delete_id(op.args[1]);
      return false;
    }
    break;

  case spv::OpBitcast:
    if (_deleted_access_chains.count(op.args[2])) {
      op.args[2] = _deleted_access_chains[op.args[2]];

      Definition &def = _db.modify_definition(op.args[1]);
      def._origin_id = op.args[2];
    }
    nassertr(!is_deleted(op.args[2]), true);

    if (!_db.get_definition(op.args[0]).is_pointer_type()) {
      mark_used(op.args[1]);
    }
    break;

  case spv::OpSelect:
    mark_used(op.args[3]);
    mark_used(op.args[4]);
    break;

  case spv::OpReturnValue:
    mark_used(op.args[0]);
    break;

  case spv::OpCopyLogical:
    // Can't copy pointers using this instruction.
    nassertr(!is_deleted(op.args[2]), true);
    nassertr(!_deleted_access_chains.count(op.args[2]), true);
    break;

  case spv::OpPtrEqual:
  case spv::OpPtrNotEqual:
  case spv::OpPtrDiff:
    mark_used(op.args[2]);
    mark_used(op.args[3]);
    break;

  default:
    return SpirVTransformPass::transform_function_op(op, function_id);
  }

  return true;
}
