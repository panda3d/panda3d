/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file wdxGraphicsBuffer9.I
 * @author zhao
 * @date 2012-09-29
 */

/**
 * Returns true if this particular GraphicsOutput can render directly into a
 * texture, or false if it must always copy-to-texture at the end of each
 * frame to achieve this effect.
 */
INLINE bool wdxGraphicsBuffer9::
get_supports_render_texture() const {
  // DX9 buffers can always bind-to-texture.
  return true;
}
