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
#include "spirVInstructionCursor.h"

/**
 *
 */
SpirVFlattenStructPass::
SpirVFlattenStructPass(Id type_id) : _type_id(type_id) {
}

/**
 *
 */
void SpirVFlattenStructPass::
run(SpirVModule &module) {
  {
    const ShaderType *type = module.resolve_type(_type_id);
    DCAST_INTO_V(_struct_type, type);
    _member_ids.resize(_struct_type->get_num_members(), Id());
  }

  // Find the variables of this struct type, along with the pointer types
  // that reference it.
  pvector<Id> struct_var_ids;
  pvector<Id> pointer_type_ids;
  size_t num_declarations = module.get_num_declarations();
  for (size_t i = 0; i < num_declarations; ++i) {
    Instruction op = module.get_declaration(i);
    if (op.opcode == spv::OpTypePointer && op.args.size() >= 3 &&
        op.args[2] == _type_id) {
      pointer_type_ids.push_back(Id(op.args[0]));
    }
    else if (op.opcode == spv::OpVariable && op.args.size() >= 3 &&
             module.unwrap_pointer_type(Id(op.args[0])) == _type_id) {
      struct_var_ids.push_back(Id(op.args[1]));
    }
  }

  // Replace each variable with individual variables for all its members.
  for (Id struct_var_id : struct_var_ids) {
    int struct_location = module.get_location(struct_var_id);
    std::string struct_var_name = module.get_name(struct_var_id);

    if (shader_cat.is_spam()) {
      shader_cat.spam()
        << "Removing variable " << struct_var_id << ": "
        << *_struct_type << " " << struct_var_name << "\n";
    }
    module.delete_id(struct_var_id);
    _deleted.insert(struct_var_id);

    for (size_t mi = 0; mi < _struct_type->get_num_members(); ++mi) {
      const ShaderType::Struct::Member &member = _struct_type->get_member(mi);

      // Insert a new variable for this struct member.
      Id variable_id = module.define_variable(member.type, spv::StorageClassUniformConstant);
      if (!member.name.empty()) {
        module.set_name(variable_id, member.name);
      }
      if (struct_location >= 0) {
        // Assign decorations to the individual members.
        module.decorate(variable_id, spv::DecorationLocation, (uint32_t)(struct_location + mi));
      }

      _member_ids[mi] = variable_id;
    }
  }

  // Delete the struct type and the pointer types that referenced it.
  for (Id pointer_type_id : pointer_type_ids) {
    module.delete_id(pointer_type_id);
  }
  module.delete_id(_type_id);

  // Now rebase all accesses through the struct onto the new variables.
  for (Function &function : module.modify_functions()) {
    SpirVInstructionCursor cursor(module, function);
    while (cursor.next()) {
      Instruction &op = *cursor;
      switch (op.opcode) {
      case spv::OpAccessChain:
      case spv::OpInBoundsAccessChain:
      case spv::OpPtrAccessChain:
      case spv::OpInBoundsPtrAccessChain:
        if (_deleted_access_chains.count(Id(op.args[2]))) {
          // The base of this access chain is an access chain we deleted.
          Id new_var_id = _deleted_access_chains[Id(op.args[2])];

          const ShaderType *type = module.resolve_type(Id(op.args[0]));
          Id pointer_type_id = module.define_pointer_type(type, spv::StorageClassUniformConstant);

          op.args[0] = pointer_type_id;
          op.args[2] = new_var_id;
          module.record_result(op, function.id);
        }
        else if (_deleted.count(Id(op.args[2]))) {
          uint32_t index = module.resolve_constant(Id(op.args[3]));
          nassertd(index < _member_ids.size()) continue;
          if (op.args.size() > 4) {
            // We also need to change the type if it has the wrong storage
            // class.  Just unwrap the first index.
            const ShaderType *type = module.resolve_type(Id(op.args[0]));
            Id pointer_type_id = module.define_pointer_type(type, spv::StorageClassUniformConstant);

            op.args[0] = pointer_type_id;
            op.args[2] = _member_ids[index];
            op.args.erase(op.args.begin() + 3);
            module.record_result(op, function.id);
          } else {
            // Delete the access chain entirely.
            Id result_id(op.args[1]);
            _deleted_access_chains[result_id] = _member_ids[index];
            _deleted.insert(result_id);
            module.delete_id(result_id);
          }
        }
        break;

      case spv::OpFunctionCall:
        for (size_t ai = 3; ai < op.args.size(); ++ai) {
          auto it = _deleted_access_chains.find(Id(op.args[ai]));
          if (it != _deleted_access_chains.end()) {
            op.args[ai] = it->second;
          }
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
          auto it = _deleted_access_chains.find(Id(op.args[2]));
          if (it != _deleted_access_chains.end()) {
            op.args[2] = it->second;
          }
        }
        nassertd(!_deleted.count(Id(op.args[2]))) continue;
        break;

      case spv::OpStore:
      case spv::OpAtomicStore:
      case spv::OpAtomicFlagClear:
        // Can't store the struct pointer itself (yet)
        {
          auto it = _deleted_access_chains.find(Id(op.args[0]));
          if (it != _deleted_access_chains.end()) {
            op.args[0] = it->second;
          }
        }
        nassertd(!_deleted.count(Id(op.args[0]))) continue;
        break;

      case spv::OpCopyMemory:
      case spv::OpCopyMemorySized:
        // Shouldn't be copying into or out of the struct directly.
        for (size_t ai = 0; ai < 2; ++ai) {
          auto it = _deleted_access_chains.find(Id(op.args[ai]));
          if (it != _deleted_access_chains.end()) {
            op.args[ai] = it->second;
          }
          nassertd(!_deleted.count(Id(op.args[ai]))) continue;
        }
        break;

      case spv::OpArrayLength:
      case spv::OpConvertPtrToU:
        {
          auto it = _deleted_access_chains.find(Id(op.args[2]));
          if (it != _deleted_access_chains.end()) {
            op.args[2] = it->second;
          }
        }
        nassertd(!_deleted.count(Id(op.args[2]))) continue;
        break;

      case spv::OpCopyObject:
      case spv::OpExpectKHR:
        {
          auto it = _deleted_access_chains.find(Id(op.args[2]));
          if (it != _deleted_access_chains.end()) {
            op.args[2] = it->second;

            // Copy the type since the storage class may have changed.
            op.args[0] = module.get_type_id(Id(op.args[2]));
            module.record_result(op, function.id);
          }
          else if (_deleted.count(Id(op.args[2]))) {
            // If it's just a copy of the struct pointer, delete the copy.
            Id result_id(op.args[1]);
            _deleted.insert(result_id);
            module.delete_id(result_id);
          }
        }
        break;

      case spv::OpBitcast:
        {
          auto it = _deleted_access_chains.find(Id(op.args[2]));
          if (it != _deleted_access_chains.end()) {
            op.args[2] = it->second;
          }
        }
        nassertd(!_deleted.count(Id(op.args[2]))) continue;
        break;

      case spv::OpCopyLogical:
        // Can't copy pointers using this instruction.
        nassertd(!_deleted.count(Id(op.args[2]))) continue;
        nassertd(!_deleted_access_chains.count(Id(op.args[2]))) continue;
        break;

      default:
        break;
      }
    }
  }
}
