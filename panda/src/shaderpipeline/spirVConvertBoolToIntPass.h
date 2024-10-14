/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVConvertBoolToIntPass.h
 * @author rdb
 * @date 2024-10-14
 */

#ifndef SPIRVCONVERTBOOLTOINTPASS_H
#define SPIRVCONVERTBOOLTOINTPASS_H

#include "spirVTransformPass.h"

/**
 * Converts all bool parameters to ints.
 */
class EXPCL_PANDA_SHADERPIPELINE SpirVConvertBoolToIntPass final : public SpirVTransformPass {
public:
  virtual bool transform_definition_op(Instruction op);
  virtual bool transform_function_op(Instruction op);

  void r_convert_to_bool(uint32_t to_type_id, uint32_t to_id, uint32_t from_id);

private:
  pset<uint32_t> _pointer_ids;
};

#endif
