// Filename: pallocator.h
// Created by:  drose (05Jun01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef PALLOCATOR_H
#define PALLOCATOR_H

#include "dtoolbase.h"

#include <memory>
#include <vector>

////////////////////////////////////////////////////////////////////
//       Class : pallocator
// Description : This is our own Panda specialization on the default
//               STL allocator.  Its main purpose is to call the hooks
//               for MemoryUsage to properly track STL-allocated
//               memory.
//
//               pvector, pmap, etc. are all defined in this directory
//               to use a pallocator.
////////////////////////////////////////////////////////////////////

#ifdef GCC_STYLE_ALLOCATOR
// Early versions of gcc used its own kind of allocator, somewhat
// different from the STL standard.

template<class Type>
class pallocator : public alloc {
public:
#ifndef NDEBUG
  static void *allocate(size_t n);
  static void deallocate(void *p, size_t n);
#endif  // NDEBUG
};

#else  // GCC_STYLE_ALLOCATOR

template<class Type>
class pallocator : public allocator<Type> {
public:
#ifndef NDEBUG
  INLINE pointer allocate(size_type n, allocator<void>::const_pointer hint = 0);
  INLINE void deallocate(pointer p, size_type n);
#endif  // NDEBUG
};
#endif  // GCC_STYLE_ALLOCATOR

#include "pallocator.T"

#endif

