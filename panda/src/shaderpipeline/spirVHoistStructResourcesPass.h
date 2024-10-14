/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVHoistStructResourcesPass.h
 * @author rdb
 * @date 2024-10-08
 */

#ifndef SPIRVHOISTSTRUCTRESOURCESPASS_H
#define SPIRVHOISTSTRUCTRESOURCESPASS_H

#include "spirVTransformPass.h"

/**
 * Moves all opaque types (and arrays of opaque types, etc.) inside structs
 * outside the structs.
 */
class EXPCL_PANDA_SHADERPIPELINE SpirVHoistStructResourcesPass final : public SpirVTransformPass {
public:
  SpirVHoistStructResourcesPass(bool remove_empty_structs) :
    _remove_empty_structs(remove_empty_structs) {}

  virtual bool transform_definition_op(Instruction op);
  virtual bool begin_function(Instruction op);
  virtual bool transform_function_op(Instruction op);

  virtual void postprocess();

private:
  // Remove structs that became empty due to only containing resources.
  const bool _remove_empty_structs;

  // Which type we need to hoist.
  pset<uint32_t> _hoist_types;

  // This stores the type IDs of all the types that (indirectly) contain the
  // type we want to unpack.  For each affected struct, access chains (struct
  // members only) leading to the hoisted type in question, as well as the
  // type that the wrapped additional variables should have.
  pmap<uint32_t, pvector<std::pair<const ShaderType *, AccessChain> > > _affected_types;
  pset<uint32_t> _affected_pointer_types;

public:
  // For each access chain consisting only of struct members
  // (prefixed by a variable id), map to the variable that has been hoisted
  pmap<AccessChain, uint32_t> _hoisted_vars;
};

#endif
