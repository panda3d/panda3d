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

/**
 *
 */
SpirVMakeBlockPass::
SpirVMakeBlockPass(const ShaderType::Struct *block_type, const pvector<uint32_t> &member_ids,
                   spv::StorageClass storage_class, uint32_t binding, uint32_t set) :
  _block_type(block_type),
  _storage_class(storage_class),
  _binding(binding),
  _set(set) {
  nassertv(block_type->get_num_members() == member_ids.size());

  for (uint32_t i = 0; i < (uint32_t)member_ids.size(); ++i) {
    uint32_t member_id = member_ids[i];
    if (member_id > 0) {
      _member_indices[member_id] = i;
      delete_id(member_id);
    }
  }
}

/**
 *
 */
bool SpirVMakeBlockPass::
begin_function(Instruction op) {
  // Define the block type at the first variable definition.
  if (_block_var_id == 0) {
    uint32_t block_type_id = define_type(_block_type);
    _block_var_id = define_variable(_block_type, _storage_class);

    decorate(block_type_id, spv::DecorationBlock);

    if (_storage_class != spv::StorageClassPushConstant) {
      decorate(_block_var_id, spv::DecorationBinding, _binding);
      decorate(_block_var_id, spv::DecorationDescriptorSet, _set);
    }

    for (auto &item : _member_indices) {
      const std::string &name = _db.get_definition(item.first)._name;
      if (!name.empty()) {
        set_member_name(block_type_id, item.second, name);
      }
    }
  }
  return true;
}

/**
 *
 */
bool SpirVMakeBlockPass::
transform_function_op(Instruction op) {
  switch (op.opcode) {
  case spv::OpAccessChain:
  case spv::OpInBoundsAccessChain:
    if (_member_indices.count(op.args[2])) {
      uint32_t result_id = op.args[1];
      uint32_t member_index = _member_indices[op.args[2]];
      uint32_t constant_id = define_int_constant(member_index);

      // Get a type pointer with the correct storage class.
      uint32_t pointer_type_id = define_pointer_type(resolve_pointer_type(op.args[0]), _storage_class);

      // Prepend our new block variable to the existing access chain.
      pvector<uint32_t> new_args({pointer_type_id, result_id, _block_var_id, constant_id});
      new_args.insert(new_args.end(), op.args + 3, op.args + op.nargs);

      add_instruction(op.opcode, new_args.data(), new_args.size());
      return false;
    }
    break;

  case spv::OpFunctionCall:
    // Add access chains when passing a load of a member id to a function.
    if (op.nargs >= 3) {
      for (size_t i = 3; i < op.nargs; ++i) {
        maybe_replace_with_access_chain(op.args[i]);
      }
    }
    break;

  case spv::OpPtrEqual:
  case spv::OpPtrNotEqual:
  case spv::OpPtrDiff:
    maybe_replace_with_access_chain(op.args[3]);
    // fall through
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
    maybe_replace_with_access_chain(op.args[2]);
    break;

  case spv::OpCopyMemory:
  case spv::OpCopyMemorySized:
    maybe_replace_with_access_chain(op.args[1]);
    // fall through
  case spv::OpStore:
  case spv::OpAtomicStore:
  case spv::OpAtomicFlagClear:
  case spv::OpReturnValue:
    maybe_replace_with_access_chain(op.args[0]);
    break;

  default:
    return SpirVTransformPass::transform_function_op(op);
  }

  return true;
}

/**
 * Replaces the given id with the new access chain if the given id was one of
 * the ids that was added to the block.
 */
bool SpirVMakeBlockPass::
maybe_replace_with_access_chain(uint32_t &id) {
  auto it = _member_indices.find(id);
  if (it != _member_indices.end()) {
    id = op_access_chain(_block_var_id, {define_int_constant(it->second)});
    return true;
  }
  return false;
}
