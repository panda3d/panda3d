/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file circularAllocator.h
 * @author rdb
 * @date 2022-01-16
 */

#ifndef CIRCULARALLOCATOR_H
#define CIRCULARALLOCATOR_H

#include "pandabase.h"
#include "linkedListNode.h"
#include "pmutex.h"
#include "mutexHolder.h"
#include "patomic.h"

/**
 * An implementation of a very simple thread-safe circular allocator.
 * Contiguousness of allocations is enforced.  In order to free memory, you
 * need to call get_head() after making all the allocations that you want to
 * reclaim, then call set_tail(offset) once you are done with them.  Freeing is
 * therefore coarse and contiguous: you can't release an individual allocation,
 * in the middle, and get_head() is only meaningful at a serialized point.
 *
 * Note that this is thread-safe only in the sense that alloc() may be called
 * from several threads at once, and each caller gets its own non-overlapping
 * region.  However, it only reserves the space; external synchronization is
 * still needed to ensure subsequent writes are visible to other threads.
 * Freeing must be done by a single thread, but is thread-safe against
 * simultaneous calls to alloc().
 *
 * Note that it's not possible to fill the buffer up entirely unless the tail
 * happens to be positioned at 0.
 */
class EXPCL_PANDA_GOBJ CircularAllocator {
public:
  constexpr CircularAllocator() = default;
  INLINE explicit CircularAllocator(size_t capacity, size_t min_alignment=0);
  INLINE CircularAllocator(CircularAllocator &&from) noexcept;

  INLINE CircularAllocator &operator = (CircularAllocator &&from) noexcept;

  ptrdiff_t alloc(size_t size, size_t alignment=0);

  INLINE void reset();
  INLINE void reset(size_t capacity);

  INLINE size_t get_capacity() const;
  INLINE size_t get_size() const;
  INLINE size_t get_head() const;
  INLINE size_t get_tail() const;
  INLINE void set_tail(size_t tail);

protected:
  patomic<size_t> _head = 0;
  patomic<size_t> _tail = 0;
  size_t _capacity = 0;
  size_t _min_alignment = 0;
};

#include "circularAllocator.I"

#endif
