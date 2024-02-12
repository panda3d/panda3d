/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file memoryHook.h
 * @author drose
 * @date 2007-06-28
 */

#ifndef MEMORYHOOK_H
#define MEMORYHOOK_H

#include "dtoolbase.h"
#include "numeric_types.h"
#include "patomic.h"
#include "mutexImpl.h"
#include <map>

#ifdef __EMSCRIPTEN__
#include <emscripten/heap.h>
#endif

/**
 * This class provides a wrapper around the various possible malloc schemes
 * Panda might employ.  It also exists to allow the MemoryUsage class in Panda
 * to insert callback hooks to track the size of allocated pointers.
 *
 * The PANDA_MALLOC_* and PANDA_FREE_* macros are defined to vector through
 * through this class (except in production builds) to facilitate that.  Every
 * memory allocation call in Panda should therefore use these macros instead
 * of direct calls to malloc or free.  (C++ new and delete operators may be
 * employed for classes which inherit from MemoryBase; otherwise, use the
 * PANDA_MALLOC macros.)
 */
class EXPCL_DTOOL_DTOOLBASE MemoryHook {
public:
  constexpr MemoryHook() = default;
  MemoryHook(const MemoryHook &copy);
  virtual ~MemoryHook() = default;

  virtual void *heap_alloc_single(size_t size);
  virtual void heap_free_single(void *ptr);

  virtual void *heap_alloc_array(size_t size);
  virtual void *heap_realloc_array(void *ptr, size_t size);
  virtual void heap_free_array(void *ptr);

  INLINE void inc_heap(size_t size);
  INLINE void dec_heap(size_t size);

  bool heap_trim(size_t pad);

  constexpr static size_t get_memory_alignment() {
    return MEMORY_HOOK_ALIGNMENT;
  }

  virtual void *mmap_alloc(size_t size, bool allow_exec);
  virtual void mmap_free(void *ptr, size_t size);
  INLINE size_t get_page_size() const;
  INLINE size_t round_up_to_page_size(size_t size) const;

  virtual void mark_pointer(void *ptr, size_t orig_size, ReferenceCount *ref_ptr);

  virtual void alloc_fail(size_t attempted_size);

  INLINE static size_t get_ptr_size(void *ptr);

protected:
  patomic<size_t> _total_heap_single_size { 0u };
  patomic<size_t> _total_heap_array_size { 0u };
  patomic<size_t> _requested_heap_size { 0u };
  patomic<size_t> _total_mmap_size { 0u };

  // If the allocated heap size crosses this threshold, we call
  // overflow_heap_size().
  size_t _max_heap_size = ~(size_t)0;

  virtual void overflow_heap_size();

#ifndef __EMSCRIPTEN__
  void determine_page_size() const;
#endif

private:
#ifdef __EMSCRIPTEN__
  static const size_t _page_size = EMSCRIPTEN_PAGE_SIZE;
#else
  mutable size_t _page_size = 0;
  mutable MutexImpl _lock;
#endif
};

#include "memoryHook.I"

#endif
