// Filename: memoryHook.h
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

#ifndef MEMORYHOOK_H
#define MEMORYHOOK_H

#include "dtoolbase.h"

////////////////////////////////////////////////////////////////////
//       Class : MemoryHook
// Description : This class is provided to allow the MemoryUsage class
//               in Panda to insert callback hooks to track the size
//               of allocated pointers.  Every low-level alloc and
//               free operation (except in production builds) should
//               vector through this class to facilitate that.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOL MemoryHook {
public:
  MemoryHook();
  MemoryHook(const MemoryHook &copy);

  virtual void *heap_alloc(size_t size);
  virtual void heap_free(void *ptr);

  virtual void *mmap_alloc(size_t size, bool allow_exec);
  virtual void mmap_free(void *ptr, size_t size);
  INLINE size_t get_page_size() const;
  INLINE size_t round_up_to_page_size(size_t size) const;

  virtual void mark_pointer(void *ptr, size_t orig_size, ReferenceCount *ref_ptr);

private:
  size_t _page_size;

#ifdef DO_MEMORY_USAGE
  INLINE static void *alloc_to_ptr(void *alloc, size_t size);
  INLINE static void *ptr_to_alloc(void *ptr, size_t &size);

protected:
  size_t _total_heap_size;
  size_t _total_mmap_size;
#endif
};

#include "memoryHook.I"

#endif
