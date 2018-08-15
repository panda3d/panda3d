/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file neverFreeMemory.cxx
 * @author drose
 * @date 2007-06-14
 */

#include "neverFreeMemory.h"
#include "atomicAdjust.h"
#include "memoryHook.h"

NeverFreeMemory * TVOLATILE NeverFreeMemory::_global_ptr;

// If a page has fewer than this many bytes remaining, never mind about it.
static const size_t min_page_remaining_size = 16;

// We always allocate at least this many bytes at a time.
static const size_t min_page_size = 128 * 1024;  // 128K

/**
 *
 */
NeverFreeMemory::
NeverFreeMemory() {
  _total_alloc = 0;
  _total_used = 0;
}

/**
 *
 */
void *NeverFreeMemory::
ns_alloc(size_t size) {
  _lock.lock();

  //NB: we no longer do alignment here.  The only class that uses this is
  // DeletedBufferChain, and we can do the alignment potentially more
  // efficiently there since we don't end up overallocating as much.
  _total_used += size;

  // Look for a page that has sufficient space remaining.

  Pages::iterator pi = _pages.lower_bound(Page(nullptr, size));
  if (pi != _pages.end()) {
    // Here's a page with enough remaining space.
    Page page = (*pi);
    _pages.erase(pi);
    void *result = page.alloc(size);
    if (page._remaining >= min_page_remaining_size) {
      _pages.insert(page);
    }
    _lock.unlock();
    return result;
  }

  // We have to allocate a new page.  Allocate at least min_page_size bytes,
  // and then round that up to the next _page_size bytes.
  size_t needed_size = std::max(size, min_page_size);
  needed_size = memory_hook->round_up_to_page_size(needed_size);
  void *start = memory_hook->mmap_alloc(needed_size, false);
  _total_alloc += needed_size;

  Page page(start, needed_size);
  void *result = page.alloc(size);
  if (page._remaining >= min_page_remaining_size) {
    _pages.insert(page);
  }
  _lock.unlock();
  return result;
}

/**
 *
 */
void NeverFreeMemory::
make_global_ptr() {
  NeverFreeMemory *ptr = new NeverFreeMemory;
  void *result = AtomicAdjust::compare_and_exchange_ptr
    ((void * TVOLATILE &)_global_ptr, nullptr, (void *)ptr);
  if (result != nullptr) {
    // Someone else got there first.
    delete ptr;
  }
}
