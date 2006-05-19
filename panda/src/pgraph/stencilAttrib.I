// Filename: stencilAttrib.I
// Created by:  aignacio (18May06)
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


////////////////////////////////////////////////////////////////////
//     Function: StencilAttrib::Constructor
//       Access: Private
//  Description: Use StencilAttrib::make() to construct a new
//               StencilAttrib object.
////////////////////////////////////////////////////////////////////
INLINE StencilAttrib::
StencilAttrib() {

  _stencil_render_states [SRS_clear_value] = 0;

  _stencil_render_states [SRS_reference] = 0;

  _stencil_render_states [SRS_read_mask] = ~0;
  _stencil_render_states [SRS_write_mask] = ~0;

  _stencil_render_states [SRS_front_enable] = 0;
  _stencil_render_states [SRS_front_comparison_function] = SCF_always;
  _stencil_render_states [SRS_front_stencil_fail_operation] = SO_keep;
  _stencil_render_states [SRS_front_stencil_pass_z_fail_operation] = SO_keep;
  _stencil_render_states [SRS_front_stencil_pass_z_pass_operation] = SO_keep;

  _stencil_render_states [SRS_back_enable] = 0;
  _stencil_render_states [SRS_back_comparison_function] = SCF_always;
  _stencil_render_states [SRS_back_stencil_fail_operation] = SO_keep;
  _stencil_render_states [SRS_back_stencil_pass_z_fail_operation] = SO_keep;
  _stencil_render_states [SRS_back_stencil_pass_z_pass_operation] = SO_keep;

  _default = true;
}

////////////////////////////////////////////////////////////////////
//     Function: StencilAttrib::set_render_state
//       Access: Published
//  Description: Sets the render state.
////////////////////////////////////////////////////////////////////
INLINE void StencilAttrib::
set_render_state(unsigned int render_state_identifier, unsigned int render_state) {
  _stencil_render_states [render_state_identifier] = render_state;
  _default = false;
}

////////////////////////////////////////////////////////////////////
//     Function: StencilAttrib::get_render_state
//       Access: Published
//  Description: Returns render state.
////////////////////////////////////////////////////////////////////
INLINE unsigned int StencilAttrib::
get_render_state(unsigned int render_state_identifier) const {
  return _stencil_render_states [render_state_identifier];
}
