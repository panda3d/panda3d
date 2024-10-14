/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVMakeBlockPass.h
 * @author rdb
 * @date 2024-10-11
 */

#ifndef SPIRVMAKEBLOCKPASS_H
#define SPIRVMAKEBLOCKPASS_H

#include "spirVTransformPass.h"

/**
 * Creates a new uniform (or push constant) block using the parameters specified
 * by the given ids and types.  This is the opposite of SpirVFlattenStructPass.
 */
class EXPCL_PANDA_SHADERPIPELINE SpirVMakeBlockPass final : public SpirVTransformPass {
public:
  SpirVMakeBlockPass(const ShaderType::Struct *block_type, const pvector<uint32_t> &member_ids,
                     spv::StorageClass storage_class, uint32_t binding=0, uint32_t set=0);

  virtual bool begin_function(Instruction op);
  virtual bool transform_function_op(Instruction op);

  bool maybe_replace_with_access_chain(uint32_t &id);

private:
  const ShaderType::Struct *_block_type;
  const spv::StorageClass _storage_class;
  const uint32_t _binding;
  const uint32_t _set;

  // Map from id to index of the member in the struct.
  pmap<uint32_t, uint32_t> _member_indices;

public:
  uint32_t _block_var_id = 0;
};

#endif
