/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file neverFreeMemory.h
 * @author drose
 * @date 2007-06-14
 */

#ifndef NEVERFREEMEMORY_H
#define NEVERFREEMEMORY_H

#include "dtoolbase.h"

#include "mutexImpl.h"
#include <set>

/**
 * This class is used to allocate bytes of memory from a pool that is never
 * intended to be freed.  It is particularly useful to support DeletedChain,
 * which allocates memory in just such a fashion.
 *
 * When it is known that memory will not be freed, it is preferable to use
 * this instead of the standard malloc() (or global_operator_new()) call,
 * since this will help reduce fragmentation problems in the dynamic heap.
 * Also, memory allocated from here will exhibit less wasted space.
 */
class EXPCL_DTOOL_DTOOLBASE NeverFreeMemory {
private:
  NeverFreeMemory();

public:
  INLINE static void *alloc(size_t size);

PUBLISHED:
  INLINE static size_t get_total_alloc();
  INLINE static size_t get_total_used();
  INLINE static size_t get_total_unused();

private:
  void *ns_alloc(size_t size);
  INLINE static NeverFreeMemory *get_global_ptr();
  static void make_global_ptr();

private:
  class Page {
  public:
    INLINE Page(void *start, size_t size);
    INLINE bool operator < (const Page &other) const;
    INLINE void *alloc(size_t size);

    unsigned char *_next;
    size_t _remaining;
  };

  typedef std::set<Page> Pages;
  Pages _pages;

  size_t _total_alloc;
  size_t _total_used;
  MutexImpl _lock;
  static NeverFreeMemory * TVOLATILE _global_ptr;
};

#include "neverFreeMemory.I"

#endif
