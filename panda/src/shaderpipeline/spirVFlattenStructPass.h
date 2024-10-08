/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVFlattenStructPass.h
 * @author rdb
 * @date 2024-10-08
 */

#ifndef SPIRVFLATTENSTRUCTPASS_H
#define SPIRVFLATTENSTRUCTPASS_H

#include "spirVTransformPass.h"

/**
 * Converts the members of the struct type with the given ID to regular
 * variables.  Useful for unwrapping uniform blocks.
 */
class EXPCL_PANDA_SHADERPIPELINE SpirVFlattenStructPass final : public SpirVTransformPass {
public:
  SpirVFlattenStructPass(uint32_t type_id);

  virtual void preprocess();

  virtual bool transform_definition_op(Instruction op);
  virtual bool transform_function_op(Instruction op, uint32_t function_id);

private:
  const uint32_t _type_id;
  const ShaderType::Struct *_struct_type = nullptr;

  // Maps access chains accessing struct members to the created variable IDs
  // for that struct member.
  pmap<uint32_t, uint32_t> _deleted_access_chains;

  // Holds the new variable IDs for each of the struct members.
  pvector<uint32_t> _member_ids;
};

#endif
