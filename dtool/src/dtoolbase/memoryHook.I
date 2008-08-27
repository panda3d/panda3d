// Filename: memoryHook.I
// Created by:  drose (28Jun07)
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


////////////////////////////////////////////////////////////////////
//     Function: MemoryHook::inc_heap
//       Access: Public
//  Description: Called by our alternative malloc implementations
//               (dlmalloc and ptmalloc2) to indicate they have
//               requested size bytes from the system for the heap.
////////////////////////////////////////////////////////////////////
INLINE void MemoryHook::
inc_heap(size_t size) {
#ifdef DO_MEMORY_USAGE
  AtomicAdjust::add(_requested_heap_size, (AtomicAdjust::Integer)size);
#endif  // DO_MEMORY_USAGE
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryHook::dec_heap
//       Access: Public
//  Description: Called by our alternative malloc implementations
//               (dlmalloc and ptmalloc2) to indicate they have
//               returned size bytes to the system from the heap.
////////////////////////////////////////////////////////////////////
INLINE void MemoryHook::
dec_heap(size_t size) {
#ifdef DO_MEMORY_USAGE
  //assert((int)size <= _requested_heap_size);
  AtomicAdjust::add(_requested_heap_size, -(AtomicAdjust::Integer)size);
#endif  // DO_MEMORY_USAGE
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryHook::get_page_size
//       Access: Public
//  Description: Returns the operating system page size.  This is the
//               minimum granularity required for calls to
//               mmap_alloc().  Also see round_up_to_page_size().
////////////////////////////////////////////////////////////////////
INLINE size_t MemoryHook::
get_page_size() const {
  return _page_size;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryHook::round_up_to_page_size
//       Access: Public
//  Description: Rounds the indicated size request up to the next
//               larger multiple of page_size, to qualify it for a
//               call to mmap_alloc().
////////////////////////////////////////////////////////////////////
INLINE size_t MemoryHook::
round_up_to_page_size(size_t size) const {
  return  ((size + _page_size - 1) / _page_size) * _page_size;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryHook::inflate_size
//       Access: Private, Static
//  Description: Increments the amount of requested size as necessary
//               to accomodate the extra data we might piggyback on
//               each allocated block.
////////////////////////////////////////////////////////////////////
INLINE size_t MemoryHook::
inflate_size(size_t size) {
#ifdef DO_MEMORY_USAGE
  return size + sizeof(size_t);
#else
  return size;
#endif  // DO_MEMORY_USAGE
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryHook::alloc_to_ptr
//       Access: Private, Static
//  Description: Converts an allocated pointer to a pointer returnable
//               to the application.  Stuffs size in the first n bytes
//               of the allocated space.
////////////////////////////////////////////////////////////////////
INLINE void *MemoryHook::
alloc_to_ptr(void *alloc, size_t size) {
#ifdef DO_MEMORY_USAGE
  size_t *root = (size_t *)alloc;
  root[0] = size;
  return (void *)(root + 1);
#else
  return alloc;
#endif  // DO_MEMORY_USAGE
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryHook::ptr_to_alloc
//       Access: Private, Static
//  Description: Converts an application pointer back to the original
//               allocated pointer.  Extracts size from the first n
//               bytes of the allocated space.
////////////////////////////////////////////////////////////////////
INLINE void *MemoryHook::
ptr_to_alloc(void *ptr, size_t &size) {
#ifdef DO_MEMORY_USAGE
  size_t *root = (size_t *)ptr;
  root -= 1;
  size = root[0];
  return (void *)root;
#else
  return ptr;
#endif  // DO_MEMORY_USAGE
}
