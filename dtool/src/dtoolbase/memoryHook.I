// Filename: memoryHook.I
// Created by:  drose (28Jun07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


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

#ifdef DO_MEMORY_USAGE
////////////////////////////////////////////////////////////////////
//     Function: MemoryHook::alloc_to_ptr
//       Access: Private
//  Description: Converts an allocated pointer to a pointer returnable
//               to the application.  Stuffs size in the first n bytes
//               of the allocated space.
////////////////////////////////////////////////////////////////////
INLINE void *MemoryHook::
alloc_to_ptr(void *alloc, size_t size) {
  size_t *root = (size_t *)alloc;
  root[0] = size;
  return (void *)(root + 1);
}
#endif  // DO_MEMORY_USAGE

#ifdef DO_MEMORY_USAGE
////////////////////////////////////////////////////////////////////
//     Function: MemoryHook::ptr_to_alloc
//       Access: Private
//  Description: Converts an application pointer back to the original
//               allocated pointer.  Extracts size from the first n
//               bytes of the allocated space.
////////////////////////////////////////////////////////////////////
INLINE void *MemoryHook::
ptr_to_alloc(void *ptr, size_t &size) {
  size_t *root = (size_t *)ptr;
  root -= 1;
  size = root[0];
  return (void *)root;
}
#endif  // DO_MEMORY_USAGE
