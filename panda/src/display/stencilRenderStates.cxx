// Filename: stencilRenderStates.cxx
// Created by:  aignacio (17May06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2006, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "graphicsStateGuardian.h"
#include "stencilRenderStates.h"

StencilRenderStates::
StencilRenderStates (GraphicsStateGuardian *gsg) {

  int index;

  // clear all
  for (index = 0; index < SRS_total; index++) {
    _stencil_render_state_array [index] = 0;
    _stencil_function_array [index] = 0;
  }

  // set default render states
  set_stencil_render_state (false, SRS_clear_value, 0);

  set_stencil_render_state (false, SRS_reference, 0);

  set_stencil_render_state (false, SRS_read_mask, ~0);
  set_stencil_render_state (false, SRS_write_mask, ~0);

  set_stencil_render_state (false, SRS_front_enable, 0);
  set_stencil_render_state (false, SRS_front_comparison_function, SCF_always);
  set_stencil_render_state (false, SRS_front_stencil_fail_operation, SO_keep);
  set_stencil_render_state (false, SRS_front_stencil_pass_z_fail_operation, SO_keep);
  set_stencil_render_state (false, SRS_front_stencil_pass_z_pass_operation, SO_keep);

  set_stencil_render_state (false, SRS_back_enable, 0);
  set_stencil_render_state (false, SRS_back_comparison_function, SCF_always);
  set_stencil_render_state (false, SRS_back_stencil_fail_operation, SO_keep);
  set_stencil_render_state (false, SRS_back_stencil_pass_z_fail_operation, SO_keep);
  set_stencil_render_state (false, SRS_back_stencil_pass_z_pass_operation, SO_keep);

  _gsg = gsg;
}

StencilRenderStates::
~StencilRenderStates (void) {

}

void StencilRenderStates::
set_stencil_render_state (bool execute_function, StencilRenderStates::StencilRenderState stencil_render_state, StencilType value) {

  _stencil_render_state_array [stencil_render_state] = value;

  if (execute_function) {
    StencilFunction stencil_function;

    stencil_function = _stencil_function_array [stencil_render_state];
    if (stencil_function) {
      stencil_function (stencil_render_state, this);
    }
  }
}

StencilType StencilRenderStates::
get_stencil_render_state (StencilRenderStates::StencilRenderState stencil_render_state) {
  return _stencil_render_state_array [stencil_render_state];
}

void StencilRenderStates::
set_stencil_function (StencilRenderStates::StencilRenderState stencil_render_state, StencilFunction stencil_function) {
  _stencil_function_array [stencil_render_state] = stencil_function;
}

