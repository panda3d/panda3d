// Filename: stencilRenderStates.h
// Created by:  aignacio (17May06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef STENCILRENDERSTATES_H
#define STENCILRENDERSTATES_H

class GraphicsStateGuardian;
typedef unsigned int StencilType;

////////////////////////////////////////////////////////////////////
//       Class : StencilRenderStates
// Description : An abstract cross-platform class for setting stencil
//               buffer render states.  Each gsg needs to create its
//               own low-level API specific functions on how to set
//               each render state. The "set_stencil_render_state"
//               function can be used in an immediate-mode fashion.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_DISPLAY StencilRenderStates {

PUBLISHED:
  enum StencilRenderState
  {
    SRS_front_enable,
    SRS_back_enable,

    SRS_front_comparison_function,
    SRS_front_stencil_fail_operation,
    SRS_front_stencil_pass_z_fail_operation,
    SRS_front_stencil_pass_z_pass_operation,

    SRS_reference,
    SRS_read_mask,
    SRS_write_mask,

    SRS_back_comparison_function,
    SRS_back_stencil_fail_operation,
    SRS_back_stencil_pass_z_fail_operation,
    SRS_back_stencil_pass_z_pass_operation,

    SRS_clear,
    SRS_clear_value,

    SRS_total,

    SRS_first = 0,
  };

  enum StencilComparisonFunction
  {
    SCF_never,
    SCF_less_than,
    SCF_equal,
    SCF_less_than_or_equal,
    SCF_greater_than,
    SCF_not_equal,
    SCF_greater_than_or_equal,
    SCF_always,
  };

  enum StencilOperation
  {
    SO_keep,
    SO_zero,
    SO_replace,
    SO_increment,
    SO_decrement,
    SO_invert,
    SO_increment_saturate,
    SO_decrement_saturate,
  };

public:
  typedef void (*StencilFunction) (StencilRenderStates::StencilRenderState stencil_render_state, StencilRenderStates *stencil_render_states);

  StencilRenderStates (GraphicsStateGuardian *gsg);
  ~StencilRenderStates (void);

  void set_stencil_render_state (bool execute_function, StencilRenderStates::StencilRenderState stencil_render_state, StencilType value);
  StencilType get_stencil_render_state (StencilRenderStates::StencilRenderState stencil_render_state);

  void set_stencil_function (StencilRenderStates::StencilRenderState stencil_render_state, StencilFunction stencil_function);

  GraphicsStateGuardian *_gsg;

private:
  StencilType _stencil_render_state_array [SRS_total];
  StencilFunction _stencil_function_array [SRS_total];
};

#endif
