/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file memoryHook.I
 * @author drose
 * @date 2007-06-28
 */

/**
 * Called by our alternative malloc implementations (dlmalloc and ptmalloc2)
 * to indicate they have requested size bytes from the system for the heap.
 */
INLINE void MemoryHook::
inc_heap(size_t size) {
#ifdef DO_MEMORY_USAGE
  AtomicAdjust::add(_requested_heap_size, (AtomicAdjust::Integer)size);
#endif  // DO_MEMORY_USAGE
}

/**
 * Called by our alternative malloc implementations (dlmalloc and ptmalloc2)
 * to indicate they have returned size bytes to the system from the heap.
 */
INLINE void MemoryHook::
dec_heap(size_t size) {
#ifdef DO_MEMORY_USAGE
  // assert((int)size <= _requested_heap_size);
  AtomicAdjust::add(_requested_heap_size, -(AtomicAdjust::Integer)size);
#endif  // DO_MEMORY_USAGE
}

/**
 * Returns the operating system page size.  This is the minimum granularity
 * required for calls to mmap_alloc().  Also see round_up_to_page_size().
 */
INLINE size_t MemoryHook::
get_page_size() const {
  return _page_size;
}

/**
 * Rounds the indicated size request up to the next larger multiple of
 * page_size, to qualify it for a call to mmap_alloc().
 */
INLINE size_t MemoryHook::
round_up_to_page_size(size_t size) const {
  return  ((size + _page_size - 1) / _page_size) * _page_size;
}

/**
 * Given a pointer that was returned by a MemoryHook allocation, returns the
 * number of bytes that were allocated for it.  This may be slightly larger
 * than the number of bytes requested.
 * The behavior of this function is undefined if the given pointer was not
 * returned by the MemoryHook allocator or was already freed.
 * May return 0 if not compiling with DO_MEMORY_USAGE.
 *
 * This is only defined publicly so TypeHandle can get at it; it really
 * shouldn't be used outside of dtoolbase.
 */
INLINE size_t MemoryHook::
get_ptr_size(void *ptr) {
#if defined(MEMORY_HOOK_DO_ALIGN)
  uintptr_t *root = (uintptr_t *)ptr;
  return (size_t)root[-2];
#elif defined(USE_MEMORY_DLMALLOC) || defined(USE_MEMORY_PTMALLOC2)
  // If we are using dlmalloc, we know how it stores the size.
  size_t *root = (size_t *)ptr;
  return (root[-1] & ~0x7) - sizeof(size_t);
#elif defined(DO_MEMORY_USAGE)
  size_t *root = (size_t *)((char *)ptr - MEMORY_HOOK_ALIGNMENT);
  return *root;
#else
  return 0;
#endif  // DO_MEMORY_USAGE
}
