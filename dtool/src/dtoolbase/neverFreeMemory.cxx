// Filename: neverFreeMemory.cxx
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

#include "neverFreeMemory.h"
#include "atomicAdjust.h"
#include "memoryHook.h"

NeverFreeMemory * TVOLATILE NeverFreeMemory::_global_ptr;

// If a page has fewer than this many bytes remaining, never mind
// about it.
static const size_t min_page_remaining_size = 16;

// We always allocate at least this many bytes at a time.
static const size_t min_page_size = 128 * 1024;  // 128K

////////////////////////////////////////////////////////////////////
//     Function: NeverFreeMemory::Constructor
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
NeverFreeMemory::
NeverFreeMemory() {
  _total_alloc = 0;
  _total_used = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NeverFreeMemory::ns_alloc
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void *NeverFreeMemory::
ns_alloc(size_t size) {
  _lock.acquire();

  // We always allocate integer multiples of this many bytes, to
  // guarantee this minimum alignment.
  static const size_t alignment_size = MemoryHook::get_memory_alignment();

  // Round up to the next alignment_size.
  size = ((size + alignment_size - 1) / alignment_size) * alignment_size;

  _total_used += size;
  
  // Look for a page that has sufficient space remaining.

  Pages::iterator pi = _pages.lower_bound(Page(NULL, size));
  if (pi != _pages.end()) {
    // Here's a page with enough remaining space.
    Page page = (*pi);
    _pages.erase(pi);
    void *result = page.alloc(size);
    if (page._remaining >= min_page_remaining_size) {
      _pages.insert(page);
    }
    _lock.release();
    return result;
  }

  // We have to allocate a new page.  Allocate at least min_page_size
  // bytes, and then round that up to the next _page_size bytes.
  size_t needed_size = max(size, min_page_size);
  needed_size = memory_hook->round_up_to_page_size(needed_size);
  void *start = memory_hook->mmap_alloc(needed_size, false);
  _total_alloc += needed_size;

  Page page(start, needed_size);
  void *result = page.alloc(size);
  if (page._remaining >= min_page_remaining_size) {
    _pages.insert(page);
  }
  _lock.release();
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: NeverFreeMemory::make_global_ptr
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
void NeverFreeMemory::
make_global_ptr() {
  NeverFreeMemory *ptr = new NeverFreeMemory;
  void *result = AtomicAdjust::compare_and_exchange_ptr
    ((void * TVOLATILE &)_global_ptr, (void *)NULL, (void *)ptr);
  if (result != NULL) {
    // Someone else got there first.
    delete ptr;
  }
}

