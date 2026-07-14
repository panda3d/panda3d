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

  SpirVInjectAlphaTestPass(Mode mode, int ref_location = -1, bool spec_constant = false) :
    _mode(mode), _ref_location(ref_location), _spec_constant(spec_constant) {}

  virtual void run(SpirVModule &module) override;
  virtual std::string_view get_name() const override { return "SpirVInjectAlphaTestPass"; }

public:
  const Mode _mode;
  const int _ref_location;
  const bool _spec_constant;

  Id _alpha_ref_var_id;

  // The result ids of the injected comparison ops, one per return statement.
  pvector<Id> _compare_op_ids;
};

#endif
