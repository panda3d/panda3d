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
INLINE size_t MemoryHook::
get_memory_alignment() {
#ifdef LINMATH_ALIGN
  // We require 16-byte alignment of certain structures, to support SSE2.  We
  // don't strictly have to align *everything*, but it's just easier to do so.
#ifdef __AVX__
  // Eigen requires 32-byte alignment when using AVX instructions.
  const size_t alignment_size = 32;
#else
  const size_t alignment_size = 16;
#endif
#else
  // Otherwise, align to two words.  This seems to be pretty standard to the
  // point where some code may rely on this being the case.
  const size_t alignment_size = sizeof(void *) * 2;
#endif
  return alignment_size;
}

/**
 * Returns the number of additional bytes that are reserved at the beginning
 * of every allocated block to store a size_t.
 */
INLINE size_t MemoryHook::
get_header_reserved_bytes() {
  // We need to figure out the minimum amount of additional space we need in
  // order to place a single word at the start of each allocated block, to
  // store the size of the block.

#ifdef LINMATH_ALIGN
  // If we're doing SSE2 alignment, we must reserve a full 16-byte block,
  // since anything less than that will spoil the alignment.
#ifdef __AVX__
  // Eigen requires 32-byte alignment when using AVX instructions.
  static const size_t header_reserved_bytes = 32;
#else
  static const size_t header_reserved_bytes = 16;
#endif

#elif defined(MEMORY_HOOK_DO_ALIGN)
  // If we're just aligning to words, we reserve a block as big as two words,
  // to allow us wiggle room to align the word precisely within that block.
  static const size_t header_reserved_bytes = sizeof(size_t) + sizeof(size_t);

#else
  // Virtually all allocators align to two words, so we make sure we preserve
  // that alignment for the benefit of anyone who relies upon that.
  static const size_t header_reserved_bytes = sizeof(void *) * 2;
#endif

  return header_reserved_bytes;
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
 * Increments the amount of requested size as necessary to accommodate the
 * extra data we might piggyback on each allocated block.
 */
INLINE size_t MemoryHook::
inflate_size(size_t size) {
#if defined(MEMORY_HOOK_DO_ALIGN)
  // If we're aligning, we need to request the header size, plus extra bytes
  // to give us wiggle room to adjust the pointer.
  return size + get_header_reserved_bytes() + get_memory_alignment() - 1;
#elif defined(DO_MEMORY_USAGE)
  // If we're not aligning, but we're tracking memory allocations, we just
  // need the header size extra (this gives us a place to store the size of
  // the allocated block).
  return size + get_header_reserved_bytes();
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
  size_t alignment = get_memory_alignment();
  // Move the allocated pointer up to the next even alignment.
  size_t *root = (size_t *)((((size_t)alloc + alignment - 1) / alignment) * alignment);
  assert(alloc <= root && (size_t)((char *)root - (char *)alloc) < alignment);
  root[0] = size;
  root[1] = (size_t)alloc;  // Save the pointer we originally allocated.
  return (void *)((char *)root + get_header_reserved_bytes());
#elif defined(DO_MEMORY_USAGE)
  size_t *root = (size_t *)alloc;
  root[0] = size;
  return (void *)((char *)root + get_header_reserved_bytes());
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
  size_t *root = (size_t *)((char *)ptr - get_header_reserved_bytes());
  size = root[0];
  void *alloc = (void *)root[1]; // Get the pointer we originally allocated.
  assert(alloc <= root && (size_t)((char *)root - (char *)alloc) < get_memory_alignment());
  return alloc;
#elif defined(DO_MEMORY_USAGE)
  size_t *root = (size_t *)((char *)ptr - get_header_reserved_bytes());
  size = root[0];
  return (void *)root;
#else
  return ptr;
#endif  // DO_MEMORY_USAGE
}
