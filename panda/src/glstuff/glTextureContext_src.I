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
  PreparedGraphicsObjects *pgo, Texture *tex) :
  TextureContext(pgo, tex),
  _num_views(0)
{
  _glgsg = glgsg;

  _index = 0;
  _indices = &_index;

  _buffer = 0;
  _buffers = nullptr;

  _has_storage = false;
  _immutable = false;
  _may_reload_with_mipmaps = false;
  _generate_mipmaps = false;
  _internal_format = 0;
  _width = 0;
  _height = 0;
  _depth = 0;
  _num_levels = 0;
  _target = GL_NONE;
}

/**
 * Returns the index for the given view of the texture.
 */
INLINE GLuint CLP(TextureContext)::
get_view_index(int view) const {
  return _indices[std::min(std::max(view, 0), _num_views - 1)];
}

/**
 * Returns the buffer index for the given view of the texture.
 */
INLINE GLuint CLP(TextureContext)::
get_view_buffer(int view) const {
  if (_buffers != nullptr) {
    return _buffers[std::min(std::max(view, 0), _num_views - 1)];
  } else {
    return 0;
  }
}
