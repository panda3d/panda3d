/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glTextureContext_src.cxx
 * @author drose
 * @date 1999-10-07
 */

#include "pnotify.h"

TypeHandle CLP(TextureContext)::_type_handle;

/**
 *
 */
CLP(TextureContext)::
~CLP(TextureContext)() {
  // Don't call glDeleteTextures; we may not have an active context.
}

/**
 * Evicts the page from the LRU.  Called internally when the LRU determines
 * that it is full.  May also be called externally when necessary to
 * explicitly evict the page.
 *
 * It is legal for this method to either evict the page as requested, do
 * nothing (in which case the eviction will be requested again at the next
 * epoch), or requeue itself on the tail of the queue (in which case the
 * eviction will be requested again much later).
 */
void CLP(TextureContext)::
evict_lru() {
  dequeue_lru();

  reset_data(_target);
  update_data_size_bytes(0);
  mark_unloaded();
}

/**
 * Resets the texture object to a new one so a new GL texture object can be
 * uploaded.  This call also allows the texture target to be changed.
 */
void CLP(TextureContext)::
reset_data(GLenum target, int num_views) {
  // Free the texture resources.
  set_num_views(0);

  _target = target;

  // We still need a valid index number, though, in case we want to re-load
  // the texture later.
  set_num_views(num_views);

  _has_storage = false;
  _immutable = false;
  _may_reload_with_mipmaps = false;

#ifndef OPENGLES_1
  // Mark the texture as coherent.
  if (gl_enable_memory_barriers) {
    _glgsg->_textures_needing_fetch_barrier.erase(this);
    _glgsg->_textures_needing_image_access_barrier.erase(this);
    _glgsg->_textures_needing_update_barrier.erase(this);
    _glgsg->_textures_needing_framebuffer_barrier.erase(this);
  }
#endif
}

/**
 * Returns an implementation-defined handle or pointer that can be used
 * to interface directly with the underlying API.
 * Returns 0 if the underlying implementation does not support this.
 */
uint64_t CLP(TextureContext)::
get_native_id() const {
  return _index;
}

/**
 * Similar to get_native_id, but some implementations use a separate
 * identifier for the buffer object associated with buffer textures.
 * Returns 0 if the underlying implementation does not support this, or
 * if this is not a buffer texture.
 */
uint64_t CLP(TextureContext)::
get_native_buffer_id() const {
  return _buffer;
}

/**
 * Changes the number of views in the texture.
 */
void CLP(TextureContext)::
set_num_views(int num_views) {
  if (_num_views > num_views) {
    glDeleteTextures(_num_views - num_views, _indices + _num_views);

    if (_buffers != nullptr) {
      _glgsg->_glDeleteBuffers(_num_views - num_views, _buffers + num_views);
    }

    if (num_views <= 1) {
      _index = _indices[0];
      if (_indices != &_index) {
        delete[] _indices;
        _indices = &_index;
      }

#ifndef OPENGLES_1
      if (_buffers != nullptr) {
        _buffer = _buffers[0];
        if (_buffers != &_buffer) {
          delete[] _buffers;
          _buffers = &_buffer;
        }
        if (num_views == 0) {
          _buffers = nullptr;
        }
      }
#endif
    }
  }
  else if (_num_views == 0 && num_views == 1) {
    glGenTextures(1, &_index);
    _indices = &_index;

#ifndef OPENGLES_1
    if (_target == GL_TEXTURE_BUFFER) {
      _glgsg->_glGenBuffers(1, &_buffer);
      _buffers = &_buffer;
    }
#endif
  }
  else if (_num_views < num_views) {
    GLuint *new_indices = new GLuint[num_views];
    memcpy(new_indices, _indices, sizeof(GLuint) * _num_views);
    glGenTextures(num_views - _num_views, new_indices + _num_views);
    if (_indices != &_index) {
      delete[] _indices;
    }
    _indices = new_indices;

#ifndef OPENGLES_1
    if (_target == GL_TEXTURE_BUFFER) {
      GLuint *new_buffers = new GLuint[num_views];
      if (_buffers != nullptr) {
        memcpy(new_buffers, _buffers, sizeof(GLuint) * _num_views);
        _glgsg->_glGenBuffers(num_views - _num_views, new_buffers + _num_views);
        if (_buffers != &_buffer) {
          delete[] _buffers;
        }
      } else {
        _glgsg->_glGenBuffers(num_views, new_buffers);
      }
      _buffers = new_buffers;
    }
#endif
  }

  _num_views = num_views;
}

#ifndef OPENGLES_1
/**
 *
 */
bool CLP(TextureContext)::
needs_barrier(GLbitfield barrier) {
  if (!gl_enable_memory_barriers) {
    return false;
  }

  return (((barrier & GL_TEXTURE_FETCH_BARRIER_BIT) &&
           _glgsg->_textures_needing_fetch_barrier.count(this)))
      || (((barrier & GL_SHADER_IMAGE_ACCESS_BARRIER_BIT) &&
           _glgsg->_textures_needing_image_access_barrier.count(this)))
      || (((barrier & GL_TEXTURE_UPDATE_BARRIER_BIT) &&
           _glgsg->_textures_needing_update_barrier.count(this)))
      || (((barrier & GL_FRAMEBUFFER_BARRIER_BIT) &&
           _glgsg->_textures_needing_framebuffer_barrier.count(this)));
}

/**
 * Mark a texture as needing a memory barrier, since a non-coherent read or
 * write just happened to it.  If 'wrote' is true, it was written to.
 */
void CLP(TextureContext)::
mark_incoherent(bool wrote) {
  if (!gl_enable_memory_barriers) {
    return;
  }

  // If we only read from it, the next read operation won't need another
  // barrier, since it'll be reading the same data.
  if (wrote) {
    _glgsg->_textures_needing_fetch_barrier.insert(this);
  }

  // We could still write to it before we read from it, so we have to always
  // insert these barriers.  This could be slightly optimized so that we don't
  // issue a barrier between consecutive image reads, but that may not be
  // worth the trouble.
  _glgsg->_textures_needing_image_access_barrier.insert(this);
  _glgsg->_textures_needing_update_barrier.insert(this);
  _glgsg->_textures_needing_framebuffer_barrier.insert(this);
}

#endif  // !OPENGLES_1
