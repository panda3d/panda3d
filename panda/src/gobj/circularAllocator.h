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

/**
 * An implementation of a very simple thread-safe circular allocator.
 * Contiguousness of allocations is enforced.  In order to free memory, you
 * need to call get_head() after an allocation, then call set_tail(offset)
 * later.
 *
 * Note that it's not possible to fill the buffer up entirely unless the tail
 * happens to be positioned at 0.
 */
class EXPCL_PANDA_GOBJ CircularAllocator {
public:
  constexpr CircularAllocator() = default;
  INLINE explicit CircularAllocator(size_t capacity, size_t min_alignment=0);
  CircularAllocator(CircularAllocator &&from) noexcept = default;

  CircularAllocator &operator = (CircularAllocator &&from) noexcept = default;

  ptrdiff_t alloc(size_t size, size_t alignment=0);

  INLINE void reset();

  INLINE size_t get_capacity() const;
  INLINE size_t get_size() const;
  INLINE size_t get_head() const;
  INLINE size_t get_tail() const;
  INLINE void set_tail(size_t tail);

protected:
  AtomicAdjust::Integer _head = 0;
  AtomicAdjust::Integer _tail = 0;
  size_t _capacity = 0;
  size_t _min_alignment = 0;
};

#include "circularAllocator.I"

#endif
