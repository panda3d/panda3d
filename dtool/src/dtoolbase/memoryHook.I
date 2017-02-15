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
 * Returns the global memory alignment.  This is the number of bytes at which
 * each allocated memory pointer will be aligned.
 */
CONSTEXPR size_t MemoryHook::
get_memory_alignment() {
  return MEMORY_HOOK_ALIGNMENT;
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
 * number of bytes that were allocated for it.  Returns 0 if not compiling
 * with DO_MEMORY_USAGE.
 */
INLINE size_t MemoryHook::
get_ptr_size(void *ptr) {
#if defined(MEMORY_HOOK_DO_ALIGN)
  uintptr_t *root = (uintptr_t *)ptr;
  return (size_t)root[-2];
#elif defined(DO_MEMORY_USAGE)
  size_t *root = (size_t *)((char *)ptr - MEMORY_HOOK_ALIGNMENT);
  return *root;
#else
  return 0;
#endif  // DO_MEMORY_USAGE
}

/**
 * Increments the amount of requested size as necessary to accommodate the
 * extra data we might piggyback on each allocated block.
 */
INLINE size_t MemoryHook::
inflate_size(size_t size) {
#if defined(MEMORY_HOOK_DO_ALIGN)
  // If we're aligning, we need to request the header size, plus extra bytes
  // to give us wiggle room to adjust the pointer.
  return size + sizeof(uintptr_t) * 2 + MEMORY_HOOK_ALIGNMENT - 1;
#elif defined(DO_MEMORY_USAGE)
  // If we're not aligning, but we're tracking memory allocations, we just
  // need the header size extra (this gives us a place to store the size of
  // the allocated block).  However, we do need to make sure that any
  // alignment guarantee is kept.
  return size + MEMORY_HOOK_ALIGNMENT;
#else
  // If we're not doing any of that, we can just allocate the precise
  // requested amount.
  return size;
#endif  // DO_MEMORY_USAGE
}

/**
 * Converts an allocated pointer to a pointer returnable to the application.
 * Stuffs size in the first n bytes of the allocated space.
 */
INLINE void *MemoryHook::
alloc_to_ptr(void *alloc, size_t size) {
#if defined(MEMORY_HOOK_DO_ALIGN)
  // Add room for two uintptr_t values.
  uintptr_t *root = (uintptr_t *)((char *)alloc + sizeof(uintptr_t) * 2);
  // Align this to the requested boundary.
  root = (uintptr_t *)(((uintptr_t)root + MEMORY_HOOK_ALIGNMENT - 1) & ~(MEMORY_HOOK_ALIGNMENT - 1));
  root[-2] = size;
  root[-1] = (uintptr_t)alloc;  // Save the pointer we originally allocated.
  return (void *)root;
#elif defined(DO_MEMORY_USAGE)
  size_t *root = (size_t *)alloc;
  root[0] = size;
  return (void *)((char *)root + MEMORY_HOOK_ALIGNMENT);
#else
  return alloc;
#endif  // DO_MEMORY_USAGE
}

/**
 * Converts an application pointer back to the original allocated pointer.
 * Extracts size from the first n bytes of the allocated space.
 */
INLINE void *MemoryHook::
ptr_to_alloc(void *ptr, size_t &size) {
#if defined(MEMORY_HOOK_DO_ALIGN)
  uintptr_t *root = (uintptr_t *)ptr;
  size = root[-2];
  return (void *)root[-1]; // Get the pointer we originally allocated.
#elif defined(DO_MEMORY_USAGE)
  size_t *root = (size_t *)((char *)ptr - MEMORY_HOOK_ALIGNMENT);
  size = root[0];
  return (void *)root;
#else
  return ptr;
#endif  // DO_MEMORY_USAGE
}
