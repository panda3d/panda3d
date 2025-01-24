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

static PStatCollector _wait_async_texture_uploads_pcollector("Wait:Async Texture Uploads");

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
  cancel_pending_uploads();

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
  _texture_fetch_barrier_counter = _glgsg->_texture_fetch_barrier_counter - 1;
  _shader_image_read_barrier_counter = _glgsg->_shader_image_access_barrier_counter - 1;
  _shader_image_write_barrier_counter = _glgsg->_shader_image_access_barrier_counter - 1;
  _texture_read_barrier_counter = _glgsg->_texture_update_barrier_counter - 1;
  _texture_write_barrier_counter = _glgsg->_shader_image_access_barrier_counter - 1;
  _framebuffer_read_barrier_counter = _glgsg->_framebuffer_barrier_counter - 1;
  _framebuffer_write_barrier_counter = _glgsg->_framebuffer_barrier_counter - 1;
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
 * Returns true if the texture needs a barrier before a read or write of the
 * given kind.  If writing is false, only writes are synced, otherwise both
 * reads and writes are synced.
 */
bool CLP(TextureContext)::
needs_barrier(GLbitfield barrier, bool writing) {
  if (!gl_enable_memory_barriers) {
    return false;
  }

  if (barrier & GL_TEXTURE_FETCH_BARRIER_BIT) {
    // This is always a read, so only sync RAW.
    if (_glgsg->_texture_fetch_barrier_counter == _texture_fetch_barrier_counter) {
      return true;
    }
  }

  if (barrier & GL_SHADER_IMAGE_ACCESS_BARRIER_BIT) {
    // Sync WAR, WAW and RAW, but not RAR.
    if ((writing && _glgsg->_shader_image_access_barrier_counter == _shader_image_read_barrier_counter) ||
        (_glgsg->_shader_image_access_barrier_counter == _shader_image_write_barrier_counter)) {
      return true;
    }
  }

  if (barrier & GL_TEXTURE_UPDATE_BARRIER_BIT) {
    if ((writing && _glgsg->_texture_update_barrier_counter == _texture_read_barrier_counter) ||
        (_glgsg->_texture_update_barrier_counter == _texture_write_barrier_counter)) {
      return true;
    }
  }

  if (barrier & GL_FRAMEBUFFER_BARRIER_BIT) {
    if ((writing && _glgsg->_framebuffer_barrier_counter == _framebuffer_read_barrier_counter) ||
        (_glgsg->_framebuffer_barrier_counter == _framebuffer_write_barrier_counter)) {
      return true;
    }
  }

  return false;
}

/**
 * Mark a texture as needing a memory barrier, since an unsynchronized read or
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
    _texture_fetch_barrier_counter = _glgsg->_texture_fetch_barrier_counter;
    _shader_image_write_barrier_counter = _glgsg->_shader_image_access_barrier_counter;
    _texture_write_barrier_counter = _glgsg->_shader_image_access_barrier_counter;
    _framebuffer_write_barrier_counter = _glgsg->_framebuffer_barrier_counter;
  }

  // We could still write to it before we read from it, so we have to always
  // insert these barriers.
  _shader_image_read_barrier_counter = _glgsg->_shader_image_access_barrier_counter;
  _texture_read_barrier_counter = _glgsg->_texture_update_barrier_counter;
  _framebuffer_read_barrier_counter = _glgsg->_framebuffer_barrier_counter;
}

#endif  // !OPENGLES_1

/**
 * Returns a PBO with the given size to the pool of unused PBOs.
 */
void CLP(TextureContext)::
return_pbo(GLuint pbo, size_t size) {
  // Also triggers when the number of buffers is -1 (which effectively means
  // to always delete the buffers after use).
  if (_num_pbos > get_texture()->get_num_async_transfer_buffers() ||
      size < _pbo_size) {
    // We have too many PBOs, or this PBO is no longer of the proper
    // size, so delete it rather than returning it to the pool.
    _num_pbos--;
    _glgsg->_glDeleteBuffers(1, &pbo);
  } else {
    _unused_pbos.push_front(pbo);
  }
}

/**
 * Deletes all unused PBOs.
 */
void CLP(TextureContext)::
delete_unused_pbos() {
  if (!_unused_pbos.empty()) {
    for (GLuint pbo : _unused_pbos) {
      _glgsg->_glDeleteBuffers(1, &pbo);
    }
    _num_pbos -= (int)_unused_pbos.size();
    _unused_pbos.clear();
  }
}

/**
 * Waits for all uploads to be finished.
 */
void CLP(TextureContext)::
do_wait_pending_uploads() const {
  PStatTimer timer(_wait_async_texture_uploads_pcollector);
  do {
    _glgsg->process_pending_jobs(true);
  }
  while (is_upload_pending());
}

/**
 *
 */
void CLP(TextureContext)::
do_wait_for_unused_pbo(int limit) const {
  PStatTimer timer(_wait_async_texture_uploads_pcollector);
  do {
    _glgsg->process_pending_jobs(true);
  }
  while (_unused_pbos.empty() && _num_pbos >= limit);
}
