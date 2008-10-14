// Filename: neverFreeMemory.I
// Created by:  drose (14Jun07)
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
//     Function: NeverFreeMemory::alloc
//       Access: Public, Static
//  Description: Returns a pointer to a newly-allocated block of
//               memory of the indicated size.
////////////////////////////////////////////////////////////////////
INLINE void *NeverFreeMemory::
alloc(size_t size) {
  return get_global_ptr()->ns_alloc(size);
}

////////////////////////////////////////////////////////////////////
//     Function: NeverFreeMemory::get_total_alloc
//       Access: Published, Static
//  Description: Returns the total number of bytes consumed by all the
//               pages allocated internally by this object.
////////////////////////////////////////////////////////////////////
INLINE size_t NeverFreeMemory::
get_total_alloc() {
  return get_global_ptr()->_total_alloc;
}

////////////////////////////////////////////////////////////////////
//     Function: NeverFreeMemory::get_total_used
//       Access: Published, Static
//  Description: Returns the total number of bytes requested by the
//               application in calls to NeverFreeMemory::alloc().
////////////////////////////////////////////////////////////////////
INLINE size_t NeverFreeMemory::
get_total_used() {
  return get_global_ptr()->_total_used;
}

////////////////////////////////////////////////////////////////////
//     Function: NeverFreeMemory::get_total_unused
//       Access: Published, Static
//  Description: Returns the difference between get_total_alloc() and
//               get_total_used().  This represents bytes in allocated
//               pages that have not (yet) been used by the
//               application.
////////////////////////////////////////////////////////////////////
INLINE size_t NeverFreeMemory::
get_total_unused() {
  NeverFreeMemory *global_ptr = get_global_ptr();
  global_ptr->_lock.acquire();
  size_t total_unused = global_ptr->_total_alloc - global_ptr->_total_used;
  global_ptr->_lock.release();
  return total_unused;
}

////////////////////////////////////////////////////////////////////
//     Function: NeverFreeMemory::get_global_ptr
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE NeverFreeMemory *NeverFreeMemory::
get_global_ptr() {
  if (_global_ptr == (NeverFreeMemory *)NULL) {
    make_global_ptr();
  }
  return _global_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: NeverFreeMemory::Page::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE NeverFreeMemory::Page::
Page(void *start, size_t size) : 
  _next((unsigned char *)start),
  _remaining(size)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NeverFreeMemory::Page::operator <
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool NeverFreeMemory::Page::
operator < (const NeverFreeMemory::Page &other) const {
  return _remaining < other._remaining;
}

////////////////////////////////////////////////////////////////////
//     Function: NeverFreeMemory::Page::alloc
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void *NeverFreeMemory::Page::
alloc(size_t size) {
  assert(size <= _remaining);
  void *result = _next;
  _next += size;
  _remaining -= size;
  return result;
}
