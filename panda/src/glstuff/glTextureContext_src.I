/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glTextureContext_src.I
 * @author drose
 * @date 1999-10-07
 */

/**
 *
 */
INLINE CLP(TextureContext)::
CLP(TextureContext)(CLP(GraphicsStateGuardian) *glgsg,
  PreparedGraphicsObjects *pgo, Texture *tex, int view) :
  TextureContext(pgo, tex, view)
{
  _glgsg = glgsg;

  glGenTextures(1, &_index);
  _buffer = 0;

#ifndef OPENGLES
  _handle = 0;
  _handle_resident = false;
#endif
  _has_storage = false;
  _immutable = false;
  _uses_mipmaps = false;
  _generate_mipmaps = false;
  _internal_format = 0;
  _width = 0;
  _height = 0;
  _depth = 0;
  _target = GL_NONE;
}
