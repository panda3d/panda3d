/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVMakeBlockPass.cxx
 * @author rdb
 * @date 2024-10-11
 */

#include "spirVMakeBlockPass.h"
#include "spirVInstructionCursor.h"

/**
 *
 */
SpirVMakeBlockPass::
SpirVMakeBlockPass(const ShaderType::Struct *block_type, const pvector<Id> &member_ids,
                   spv::StorageClass storage_class, uint32_t binding, uint32_t set) :
  _block_type(block_type),
  _storage_class(storage_class),
  _binding(binding),
  _set(set) {
  nassertv(block_type->get_num_members() == member_ids.size());

  for (uint32_t i = 0; i < (uint32_t)member_ids.size(); ++i) {
    Id member_id = member_ids[i];
    if (member_id != 0) {
      _member_indices[member_id] = i;
    }
  }
}

/**
 *
 */
void SpirVMakeBlockPass::
run(SpirVModule &module) {
  // Define the block type and variable.
  Id block_type_id = module.define_type(_block_type);
  _block_var_id = module.define_variable(_block_type, _storage_class);

  module.decorate(block_type_id, spv::DecorationBlock);

  if (_storage_class != spv::StorageClassPushConstant) {
    module.decorate(_block_var_id, spv::DecorationBinding, _binding);
    module.decorate(_block_var_id, spv::DecorationDescriptorSet, _set);
  }

  for (const auto &item : _member_indices) {
    const std::string &name = module.get_name(item.first);
    if (!name.empty()) {
      module.set_member_name(block_type_id, item.second, name);
    }
  }

  // Rewrite all accesses to the old variables into accesses to the block.
  for (Function &function : module.modify_functions()) {
    SpirVInstructionCursor cursor(module, function);
    while (cursor.next()) {
      // Replaces the given operand of the instruction being visited with a
      // freshly inserted access chain, if it refers to one of the old
      // variables.
      auto replace_arg = [&](size_t arg_index) {
        if (arg_index >= cursor->args.size()) {
          return;
        }
        auto it = _member_indices.find(Id(cursor->args[arg_index]));
        if (it != _member_indices.end()) {
          Id constant_id = module.define_int_constant(it->second);
          Id chain_id;
          cursor.insert_before([&](SpirVBuilder &builder) {
            chain_id = builder.op_access_chain(_block_var_id, {constant_id});
          });
          cursor->args[arg_index] = chain_id;
        }
      };

      switch (cursor->opcode) {
      case spv::OpAccessChain:
      case spv::OpInBoundsAccessChain:
        {
          Instruction &op = *cursor;
          if (op.args.size() >= 3 && _member_indices.count(Id(op.args[2]))) {
            uint32_t member_index = _member_indices[Id(op.args[2])];
            Id constant_id = module.define_int_constant(member_index);

            // Get a type pointer with the correct storage class, and prepend
            // our new block variable to the existing access chain.
            op.args[0] = module.define_pointer_type(
              module.resolve_type(Id(op.args[0])), _storage_class);
            op.args[2] = _block_var_id;
            op.args.insert(op.args.begin() + 3, (uint32_t)constant_id);

            // The result type changed; make the id index follow.
            module.record_result(op, function.id);
          }
        }
        break;

      case spv::OpFunctionCall:
        // Add access chains when passing a load of a member id to a function.
        for (size_t ai = 3; ai < cursor->args.size(); ++ai) {
          replace_arg(ai);
        }
        break;

      case spv::OpPtrEqual:
      case spv::OpPtrNotEqual:
      case spv::OpPtrDiff:
        replace_arg(3);
        replace_arg(2);
        break;

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
      case spv::OpImageTexelPointer:
      case spv::OpCopyObject:
      case spv::OpExpectKHR:
      case spv::OpBitcast:
      case spv::OpCopyLogical:
        // Add access chains before all loads to access the right block member.
        replace_arg(2);
        break;

      case spv::OpCopyMemory:
      case spv::OpCopyMemorySized:
        replace_arg(1);
        replace_arg(0);
        break;

      case spv::OpStore:
      case spv::OpAtomicStore:
      case spv::OpAtomicFlagClear:
      case spv::OpReturnValue:
        replace_arg(0);
        break;

      default:
        break;
      }
    }
  }

  // Delete the old individual variables.
  for (const auto &item : _member_indices) {
    module.delete_id(item.first);
  }
}
