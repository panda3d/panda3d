// Filename: stencilAttrib.I
// Created by:  aignacio (18May06)
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

////////////////////////////////////////////////////////////////////
//     Function: StencilAttrib::get_render_state
//       Access: Published
//  Description: Returns render state.
////////////////////////////////////////////////////////////////////
INLINE unsigned int StencilAttrib::
get_render_state(StencilRenderState render_state_identifier) const {
  return _stencil_render_states[(int)render_state_identifier];
}
