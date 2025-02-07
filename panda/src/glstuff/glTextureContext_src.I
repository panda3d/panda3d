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

/**
 * Returns true if an async upload is pending.
 */
INLINE bool CLP(TextureContext)::
is_upload_pending() const {
  // We can't simply compare _uploads_started to _uploads_finished, since
  // they also get set to the same by cancel_pending_uploads()
  return _uploads_pending > 0;
}

/**
 * Waits for all uploads to be finished.
 */
INLINE void CLP(TextureContext)::
wait_pending_uploads() const {
  if (is_upload_pending()) {
    do_wait_pending_uploads();
  }
}

/**
 * Cancels all asynchronous uploads.  Not guaranteed to be cancelled by the
 * time this returns, consider following this up with a call to
 * wait_pending_uploads().
 */
INLINE void CLP(TextureContext)::
cancel_pending_uploads() {
  _uploads_finished = _uploads_started;
}

/**
 * Waits for an unused PBO unless we're not at the given limit of PBOs yet.
 */
INLINE void CLP(TextureContext)::
wait_for_unused_pbo(int limit) const {
  if (_unused_pbos.empty() && _num_pbos >= limit) {
    do_wait_for_unused_pbo(limit);
  }
}
