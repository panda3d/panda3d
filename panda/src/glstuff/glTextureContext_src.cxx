// Filename: glTextureContext.cxx
// Created by:  drose (07Oct99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "pnotify.h"

TypeHandle CLP(TextureContext)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CLP(TextureContext)::Denstructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CLP(TextureContext)::
~CLP(TextureContext)() {
#ifndef OPENGLES
  if (gl_enable_memory_barriers) {
    _glgsg->_textures_needing_fetch_barrier.erase(this);
    _glgsg->_textures_needing_image_access_barrier.erase(this);
    _glgsg->_textures_needing_update_barrier.erase(this);
    _glgsg->_textures_needing_framebuffer_barrier.erase(this);
  }
#endif

  glDeleteTextures(1, &_index);
  _index = 0;

  if (_buffer != 0) {
    _glgsg->_glDeleteBuffers(1, &_buffer);
    _buffer = 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLTextureContext::evict_lru
//       Access: Public, Virtual
//  Description: Evicts the page from the LRU.  Called internally when
//               the LRU determines that it is full.  May also be
//               called externally when necessary to explicitly evict
//               the page.
//
//               It is legal for this method to either evict the page
//               as requested, do nothing (in which case the eviction
//               will be requested again at the next epoch), or
//               requeue itself on the tail of the queue (in which
//               case the eviction will be requested again much
//               later).
////////////////////////////////////////////////////////////////////
void CLP(TextureContext)::
evict_lru() {
  dequeue_lru();

#ifndef OPENGLES
  if (_handle != 0) {
    if (_handle_resident) {
      _glgsg->_glMakeTextureHandleNonResident(_handle);
    }
    _handle_resident = false;
  } else
#endif
  {
    reset_data();
  }

  update_data_size_bytes(0);
  mark_unloaded();
}

////////////////////////////////////////////////////////////////////
//     Function: GLTextureContext::reset_data
//       Access: Public
//  Description: Resets the texture object to a new one so a new GL
//               texture object can be uploaded.
////////////////////////////////////////////////////////////////////
void CLP(TextureContext)::
reset_data() {
#ifndef OPENGLES
  if (_handle != 0 && _handle_resident) {
    _glgsg->_glMakeTextureHandleNonResident(_handle);
  }
#endif

  // Free the texture resources.
  glDeleteTextures(1, &_index);

  if (_buffer != 0) {
    _glgsg->_glDeleteBuffers(1, &_buffer);
    _buffer = 0;
  }

  // We still need a valid index number, though, in case we want to
  // re-load the texture later.
  glGenTextures(1, &_index);

  _handle = 0;
  _handle_resident = false;
  _has_storage = false;
  _immutable = false;

#ifndef OPENGLES
  // Mark the texture as coherent.
  if (gl_enable_memory_barriers) {
    _glgsg->_textures_needing_fetch_barrier.erase(this);
    _glgsg->_textures_needing_image_access_barrier.erase(this);
    _glgsg->_textures_needing_update_barrier.erase(this);
    _glgsg->_textures_needing_framebuffer_barrier.erase(this);
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: GLTextureContext::make_handle_resident
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(TextureContext)::
make_handle_resident() {
#ifndef OPENGLES
  if (_handle != 0) {
    if (!_handle_resident) {
      _glgsg->_glMakeTextureHandleResident(_handle);
      _handle_resident = true;
    }
    set_resident(true);
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(TextureContext)::get_handle
//       Access: Public
//  Description: Returns a handle for this texture.  Once this has
//               been created, the texture data may still be updated,
//               but its properties may not.
////////////////////////////////////////////////////////////////////
INLINE GLuint64 CLP(TextureContext)::
get_handle() {
#ifdef OPENGLES
  return 0;
#else
  if (!_glgsg->_supports_bindless_texture) {
    return false;
  }

  if (_handle == 0) {
    _handle = _glgsg->_glGetTextureHandle(_index);
  }

  _immutable = true;
  return _handle;
#endif
}

#ifndef OPENGLES
////////////////////////////////////////////////////////////////////
//     Function: GLTextureContext::needs_barrier
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: GLTextureContext::mark_incoherent
//       Access: Public
//  Description: Mark a texture as needing a memory barrier, since
//               a non-coherent read or write just happened to it.
//               If 'wrote' is true, it was written to.
////////////////////////////////////////////////////////////////////
void CLP(TextureContext)::
mark_incoherent(bool wrote) {
  if (!gl_enable_memory_barriers) {
    return;
  }

  // If we only read from it, the next read operation won't need
  // another barrier, since it'll be reading the same data.
  if (wrote) {
    _glgsg->_textures_needing_fetch_barrier.insert(this);
  }

  // We could still write to it before we read from it, so we have
  // to always insert these barriers.  This could be slightly
  // optimized so that we don't issue a barrier between consecutive
  // image reads, but that may not be worth the trouble.
  _glgsg->_textures_needing_image_access_barrier.insert(this);
  _glgsg->_textures_needing_update_barrier.insert(this);
  _glgsg->_textures_needing_framebuffer_barrier.insert(this);
}

#endif // OPENGLES
