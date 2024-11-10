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
 * The alpha test is always performed on the output with location 0.
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

  SpirVInjectAlphaTestPass(Mode mode, int ref_location = -1) :
    _mode(mode), _ref_location(ref_location) {}

  virtual bool transform_entry_point(spv::ExecutionModel model, uint32_t id, const char *name, const uint32_t *var_ids, uint16_t num_vars);
  virtual bool begin_function(Instruction op);
  virtual bool transform_function_op(Instruction op);
  virtual void end_function(uint32_t function_id);

public:
  const Mode _mode;
  const int _ref_location;

  uint32_t _alpha_ref_var_id = 0;

private:
  uint32_t _var_id = 0;

  // For each entry point we access, the output variable to test.
  pmap<uint32_t, uint32_t> _entry_points;
};

#endif
