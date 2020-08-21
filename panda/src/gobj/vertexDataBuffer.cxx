/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vertexDataBuffer.cxx
 * @author drose
 * @date 2007-05-14
 */

#include "vertexDataBuffer.h"
#include "config_gobj.h"
#include "pStatTimer.h"

TypeHandle VertexDataBuffer::_type_handle;

/**
 *
 */
void VertexDataBuffer::
operator = (const VertexDataBuffer &copy) {
  LightMutexHolder holder(_lock);
  LightMutexHolder holder2(copy._lock);

  if (_resident_data != nullptr) {
    nassertv(_reserved_size != 0);
    get_class_type().deallocate_array(_resident_data);
    _resident_data = nullptr;
  }
  if (copy._resident_data != nullptr && copy._size != 0) {
    // We only allocate _size bytes, not the full _reserved_size allocated by
    // the original copy.
    _resident_data = (unsigned char *)get_class_type().allocate_array(copy._size);
    memcpy(_resident_data, copy._resident_data, copy._size);
  }
  _size = copy._size;
  _reserved_size = copy._size;
  _block = copy._block;
  nassertv(_reserved_size >= _size);
}

/**
 * Swaps the data buffers between this one and the other one.
 */
void VertexDataBuffer::
swap(VertexDataBuffer &other) {
  LightMutexHolder holder(_lock);
  LightMutexHolder holder2(other._lock);

  unsigned char *resident_data = _resident_data;
  size_t size = _size;
  size_t reserved_size = _reserved_size;

  _block.swap(other._block);

  _resident_data = other._resident_data;
  _size = other._size;
  _reserved_size = other._reserved_size;

  other._resident_data = resident_data;
  other._size = size;
  other._reserved_size = reserved_size;
  nassertv(_reserved_size >= _size);
}

/**
 * Changes the reserved size of the buffer, preserving its data (except for
 * any data beyond the new end of the buffer, if the buffer is being reduced).
 * If the buffer is expanded, the new data is uninitialized.
 *
 * Assumes the lock is already held.
 */
void VertexDataBuffer::
do_clean_realloc(size_t reserved_size) {
  if (reserved_size != _reserved_size) {
    if (reserved_size == 0 || _size == 0) {
      do_unclean_realloc(reserved_size);
      return;
    }

    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
        << this << ".clean_realloc(" << reserved_size << ")\n";
    }

    // Page in if we're currently paged out.
    if (_reserved_size != 0 && _resident_data == nullptr) {
      do_page_in();
    }

    if (_reserved_size == 0) {
      nassertv(_resident_data == nullptr);
      _resident_data = (unsigned char *)get_class_type().allocate_array(reserved_size);
    } else {
      nassertv(_resident_data != nullptr);
      _resident_data = (unsigned char *)get_class_type().reallocate_array(_resident_data, reserved_size);
    }
    nassertv(_resident_data != nullptr);
    _reserved_size = reserved_size;
  }

  _size = std::min(_size, _reserved_size);
}

/**
 * Changes the reserved size of the buffer, without regard to preserving its
 * data.  This implicitly resets the size to 0.
 *
 * Assumes the lock is already held.
 */
void VertexDataBuffer::
do_unclean_realloc(size_t reserved_size) {
  if (reserved_size != _reserved_size || _resident_data == nullptr) {
    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
        << this << ".unclean_realloc(" << reserved_size << ")\n";
    }

    // If we're paged out, discard the page.
    _block = nullptr;

    if (_resident_data != nullptr) {
      nassertv(_reserved_size != 0);

      get_class_type().deallocate_array(_resident_data);
      _resident_data = nullptr;
      _reserved_size = 0;
    }

    if (reserved_size != 0) {
      nassertv(_resident_data == nullptr);
      _resident_data = (unsigned char *)get_class_type().allocate_array(reserved_size);
    }

    _reserved_size = reserved_size;
  }

  _size = 0;
}

/**
 * Moves the buffer out of independent memory and puts it on a page in the
 * indicated book.  The buffer may still be directly accessible as long as its
 * page remains resident.  Any subsequent attempt to rewrite the buffer will
 * implicitly move it off of the page and back into independent memory.
 *
 * Assumes the lock is already held.
 */
void VertexDataBuffer::
do_page_out(VertexDataBook &book) {
  if (_block != nullptr || _reserved_size == 0) {
    // We're already paged out.
    return;
  }
  nassertv(_resident_data != nullptr);

  if (_size == 0) {
    // It's an empty buffer.  Just deallocate it; don't bother to create a
    // block.
    get_class_type().deallocate_array(_resident_data);
    _resident_data = nullptr;
    _reserved_size = 0;

  } else {
    // It's a nonempty buffer, so write _size bytes (but not the full
    // _reserved_size bytes) to a block.
    _block = book.alloc(_size);
    nassertv(_block != nullptr);
    unsigned char *pointer = _block->get_pointer(true);
    nassertv(pointer != nullptr);
    memcpy(pointer, _resident_data, _size);

    get_class_type().deallocate_array(_resident_data);
    _resident_data = nullptr;

    _reserved_size = _size;
  }
  nassertv(_reserved_size >= _size);
}

/**
 * Moves the buffer off of its current page and into independent memory.  If
 * the page is not already resident, it is forced resident first.
 *
 * Assumes the lock is already held.
 */
void VertexDataBuffer::
do_page_in() {
  if (_resident_data != nullptr || _reserved_size == 0) {
    // We're already paged in.
    return;
  }

  nassertv(_block != nullptr);
  nassertv(_reserved_size == _size);

  _resident_data = (unsigned char *)get_class_type().allocate_array(_size);
  nassertv(_resident_data != nullptr);

  memcpy(_resident_data, _block->get_pointer(true), _size);
}
