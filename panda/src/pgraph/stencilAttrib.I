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
//     Function: StencilAttrib::get_render_state
//       Access: Published
//  Description: Returns render state.
////////////////////////////////////////////////////////////////////
INLINE unsigned int StencilAttrib::
get_render_state(unsigned int render_state_identifier) const {
  return _stencil_render_states [render_state_identifier];
}
