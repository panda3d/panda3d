// Filename: neverFreeMemory.cxx
// Created by:  drose (14Jun07)
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

#include "neverFreeMemory.h"
#include "atomicAdjust.h"

#ifdef WIN32

#else

// Posix case.
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

#endif  // WIN32

NeverFreeMemory *NeverFreeMemory::_global_ptr;

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

  _page_size = 1;

#ifdef WIN32


#else

  // Posix case.
  _page_size = getpagesize();

#endif  // WIN32
}

////////////////////////////////////////////////////////////////////
//     Function: NeverFreeMemory::ns_alloc
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void *NeverFreeMemory::
ns_alloc(size_t size) {
  _lock.lock();

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
  needed_size = ((needed_size + _page_size - 1) / _page_size) * _page_size;
  void *start = NULL;

#ifdef WIN32

#else

  // Posix case.
  start = mmap(NULL, needed_size, PROT_READ | PROT_WRITE, 
               MAP_PRIVATE | MAP_ANON, -1, 0);
  if (start == (void *)-1) {
    perror("mmap");
    start = NULL;
  }

#endif  // WIN32

  if (start == NULL) {
    start = malloc(needed_size);
  }

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
  assert(_global_ptr != (NeverFreeMemory *)NULL);
}
