// Filename: stencilRenderStates.cxx
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

#include "graphicsStateGuardian.h"
#include "stencilRenderStates.h"

////////////////////////////////////////////////////////////////////
//     Function: StencilRenderStates::Constructor
//  Description: Constructor.  All data is set to the default.
////////////////////////////////////////////////////////////////////
StencilRenderStates::
StencilRenderStates (GraphicsStateGuardian *gsg) {

  int index;

  // clear all
  for (index = 0; index < SRS_total; index++) {
    _stencil_render_state_array [index] = 0;
    _stencil_function_array [index] = 0;
  }

  // set default render states
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

////////////////////////////////////////////////////////////////////
//     Function: StencilRenderStates::Destructor
//  Description:
////////////////////////////////////////////////////////////////////
StencilRenderStates::
~StencilRenderStates (void) {

}

////////////////////////////////////////////////////////////////////
//     Function: StencilRenderStates::set_stencil_render_state
//  Description: Sets the current render state for the specified
//               stencil render state. The execute_function
//               parameter can be used to defer the actual setting
//               of the render state at the API level.  This is
//               useful for the OpenGL API where certain render
//               states are not independent/atomic (i.e.
//               glStencilFunc and glStencilOp).
////////////////////////////////////////////////////////////////////
void StencilRenderStates::
set_stencil_render_state (bool execute_function, StencilRenderStates::StencilRenderState stencil_render_state, StencilType value) {

  // DEBUG
  if (false) {
    printf ("SRS %d %d %d \n", execute_function, stencil_render_state, value);
  }

  _stencil_render_state_array [stencil_render_state] = value;

  if (execute_function) {
    StencilFunction stencil_function;

    stencil_function = _stencil_function_array [stencil_render_state];
    if (stencil_function) {
      stencil_function (stencil_render_state, this);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: StencilRenderStates::get_stencil_render_state
//  Description: Gets the current render state for the specified
//               stencil render state.
////////////////////////////////////////////////////////////////////
StencilType StencilRenderStates::
get_stencil_render_state (StencilRenderStates::StencilRenderState stencil_render_state) {
  return _stencil_render_state_array [stencil_render_state];
}

////////////////////////////////////////////////////////////////////
//     Function: StencilRenderStates::set_stencil_function
//  Description: Registers an API specific callback for setting a
//               specified stencil render state.
////////////////////////////////////////////////////////////////////
void StencilRenderStates::
set_stencil_function (StencilRenderStates::StencilRenderState stencil_render_state, StencilFunction stencil_function) {
  _stencil_function_array [stencil_render_state] = stencil_function;
}
