/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVInjectAlphaTestPass.h
 * @author rdb
 * @date 2024-11-10
 */

#ifndef SPIRVINJECTALPHATESTPASS_H
#define SPIRVINJECTALPHATESTPASS_H

#include "spirVTransformPass.h"

/**
 * Injects an alpha test before all return statements of fragment entry points.
 */
class EXPCL_PANDA_SHADERPIPELINE SpirVInjectAlphaTestPass final : public SpirVTransformPass {
public:
  enum Mode {
    M_none=0,           // alpha-test disabled (always-draw)
    M_never,            // Never draw.
    M_less,             // incoming < reference_alpha
    M_equal,            // incoming == reference_alpha
    M_less_equal,       // incoming <= reference_alpha
    M_greater,          // incoming > reference_alpha
    M_not_equal,        // incoming != reference_alpha
    M_greater_equal,    // incoming >= reference_alpha
    M_always            // Always draw.
  };

  SpirVInjectAlphaTestPass(Mode mode, int location = -1) : _mode(mode), _location(location) {}

  virtual bool transform_entry_point(spv::ExecutionModel model, uint32_t id, const char *name, const uint32_t *interface, uint16_t size);
  virtual bool begin_function(Instruction op);
  virtual bool transform_function_op(Instruction op);
  virtual void end_function(uint32_t function_id);

public:
  const Mode _mode;
  const int _location;

  uint32_t _alpha_ref_var_id = 0;

private:
  uint32_t _var_id = 0;

  // For each entry point we access, the output variable to test.
  pmap<uint32_t, uint32_t> _entry_points;

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
