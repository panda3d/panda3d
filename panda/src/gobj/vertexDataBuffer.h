/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vertexDataBuffer.h
 * @author drose
 * @date 2007-05-14
 */

#ifndef VERTEXDATABUFFER_H
#define VERTEXDATABUFFER_H

#include "pandabase.h"
#include "vertexDataBook.h"
#include "vertexDataBlock.h"
#include "pointerTo.h"
#include "virtualFile.h"
#include "pStatCollector.h"
#include "lightMutex.h"
#include "lightMutexHolder.h"

/**
 * A block of bytes that stores the actual raw vertex data referenced by a
 * GeomVertexArrayData object.
 *
 * At any point, a buffer may be in any of two states:
 *
 * independent - the buffer's memory is resident, and owned by the
 * VertexDataBuffer object itself (in _resident_data).  In this state,
 * _reserved_size might be greater than or equal to _size.
 *
 * paged - the buffer's memory is owned by a VertexDataBlock.  That block
 * might itself be resident, compressed, or paged to disk.  If it is resident,
 * the memory may still be accessed directly from the block.  However, this
 * memory is considered read-only.  In this state, _reserved_size will always
 * equal _size.
 *
 * VertexDataBuffers start out in independent state.  They get moved to paged
 * state when their owning GeomVertexArrayData objects get evicted from the
 * _independent_lru.  They can get moved back to independent state if they are
 * modified (e.g.  get_write_pointer() or realloc() is called).
 *
 * The idea is to keep the highly dynamic and frequently-modified
 * VertexDataBuffers resident in easy-to-access memory, while collecting the
 * static and rarely accessed VertexDataBuffers together onto pages, where
 * they may be written to disk as a block when necessary.
 */
class EXPCL_PANDA_GOBJ VertexDataBuffer {
public:
  INLINE VertexDataBuffer();
  INLINE VertexDataBuffer(size_t size);
  INLINE VertexDataBuffer(const VertexDataBuffer &copy);
  void operator = (const VertexDataBuffer &copy);
  INLINE ~VertexDataBuffer();

  INLINE const unsigned char *get_read_pointer(bool force) const RETURNS_ALIGNED(MEMORY_HOOK_ALIGNMENT);
  INLINE unsigned char *get_write_pointer() RETURNS_ALIGNED(MEMORY_HOOK_ALIGNMENT);

  INLINE size_t get_size() const;
  INLINE size_t get_reserved_size() const;
  INLINE void set_size(size_t size);
  INLINE void clean_realloc(size_t reserved_size);
  INLINE void unclean_realloc(size_t reserved_size);
  INLINE void clear();

  INLINE void page_out(VertexDataBook &book);

  void swap(VertexDataBuffer &other);

private:
  void do_clean_realloc(size_t size);
  void do_unclean_realloc(size_t size);

  void do_page_out(VertexDataBook &book);
  void do_page_in();

  unsigned char *_resident_data;
  size_t _size;
  size_t _reserved_size;
  PT(VertexDataBlock) _block;
  LightMutex _lock;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "VertexDataBuffer");
  }

private:
  static TypeHandle _type_handle;
};

#include "vertexDataBuffer.I"

#endif
