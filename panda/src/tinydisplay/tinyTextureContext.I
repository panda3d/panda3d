/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file tinyTextureContext.I
 * @author drose
 * @date 2008-04-30
 */

/**
 *
 */
INLINE TinyTextureContext::
TinyTextureContext(PreparedGraphicsObjects *pgo, Texture *tex, int view) :
  TextureContext(pgo, tex, view)
{
  _gltex.num_levels = 0;
  _gltex.allocated_buffer = nullptr;
  _gltex.total_bytecount = 0;
}
