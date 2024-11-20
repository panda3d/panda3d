/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVEmulateTextureQueriesPass.h
 * @author rdb
 * @date 2024-11-19
 */

#ifndef SPIRVEMULATETEXTUREQUERIESPASS_H
#define SPIRVEMULATETEXTUREQUERIESPASS_H

#include "spirVTransformPass.h"

/**
 * Emulates textureSize, imageSize and textureQueryLevels ops.
 */
class EXPCL_PANDA_SHADERPIPELINE SpirVEmulateTextureQueriesPass final : public SpirVTransformPass {
public:
  SpirVEmulateTextureQueriesPass() = default;

  virtual bool transform_definition_op(Instruction op);
  virtual bool transform_function_op(Instruction op);

private:
  pmap<uint32_t, AccessChain> _access_chains;

public:
  // access chain to size var id
  pmap<AccessChain, uint32_t> _size_var_ids;
};

#endif
