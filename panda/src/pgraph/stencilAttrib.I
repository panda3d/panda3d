/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stencilAttrib.I
 * @author aignacio
 * @date 2006-05-18
 */

/**
 * Returns render state.
 */
INLINE unsigned int StencilAttrib::
get_render_state(StencilRenderState render_state_identifier) const {
  return _stencil_render_states[(int)render_state_identifier];
}
