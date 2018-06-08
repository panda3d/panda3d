/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textureReloadRequest.I
 * @author drose
 * @date 2008-08-12
 */

/**
 * Create a new TextureReloadRequest, and add it to the loader via
 * load_async(), to begin an asynchronous load.
 */
INLINE TextureReloadRequest::
TextureReloadRequest(const std::string &name,
                     PreparedGraphicsObjects *pgo, Texture *texture,
                     bool allow_compressed) :
  AsyncTask(name),
  _pgo(pgo),
  _texture(texture),
  _allow_compressed(allow_compressed)
{
  nassertv(_pgo != nullptr);
  nassertv(_texture != nullptr);
}

/**
 * Returns the PreparedGraphicsObjects object associated with this
 * asynchronous TextureReloadRequest.
 */
INLINE PreparedGraphicsObjects *TextureReloadRequest::
get_prepared_graphics_objects() const {
  return _pgo;
}

/**
 * Returns the Texture object associated with this asynchronous
 * TextureReloadRequest.
 */
INLINE Texture *TextureReloadRequest::
get_texture() const {
  return _texture;
}

/**
 * Returns the "allow compressed" flag associated with this asynchronous
 * TextureReloadRequest.
 */
INLINE bool TextureReloadRequest::
get_allow_compressed() const {
  return _allow_compressed;
}

/**
 * Returns true if this request has completed, false if it is still pending.
 * Equivalent to `req.done() and not req.cancelled()`.
 * @see done()
 */
INLINE bool TextureReloadRequest::
is_ready() const {
  return (FutureState)AtomicAdjust::get(_future_state) == FS_finished;
}
