/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVReplaceVariableTypePass.h
 * @author rdb
 * @date 2024-10-08
 */

#ifndef SPIRVREPLACEVARIABLETYPEPASS_H
#define SPIRVREPLACEVARIABLETYPEPASS_H

#include "spirVTransformPass.h"

/**
 * Changes the type of a given variable.  Does not check that the existing
 * usage of the variable in the shader is valid with the new type - it only
 * changes the types of loads and copies.  An exception is when changing a
 * scalar or vector to a different scalar type or number of components, where
 * conversion is performed at the load point.
 */
class EXPCL_PANDA_SHADERPIPELINE SpirVReplaceVariableTypePass final : public SpirVTransformPass {
public:
  SpirVReplaceVariableTypePass(uint32_t variable_id, const ShaderType *type,
                               spv::StorageClass storage_class);

  virtual bool transform_definition_op(Instruction op);
  virtual bool transform_function_op(Instruction op);

private:
  const uint32_t _variable_id;
  const ShaderType *const _new_type;
  const spv::StorageClass _new_storage_class;

  uint32_t _pointer_type_id = 0;
  uint32_t _type_id = 0;
  pset<uint32_t> _pointer_ids;
  pset<uint32_t> _object_ids;
};

#endif
