// Filename: dallocator.h
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

#ifndef DALLOCATOR_H
#define DALLOCATOR_H

#include "dtoolbase.h"

#include <memory>

////////////////////////////////////////////////////////////////////
//       Class : dallocator
// Description : This is similar to pallocator, but always uses the
//               default new and delete handlers defined in
//               dtoolbase_cc.h; it never calls the hooks assigned by
//               redefining global_operator_new, etc.
//
//               This is needed in those rare cases when we need to
//               allocate memory for STL without going through the
//               callback hooks, for instance to implement STL tables
//               within the MemoryUsage class itself.
////////////////////////////////////////////////////////////////////

#ifdef GCC_STYLE_ALLOCATOR
// Early versions of gcc used its own kind of allocator, somewhat
// different from the STL standard.

template<class Type>
class dallocator : public alloc {
public:
#ifndef NDEBUG
  static void *allocate(size_t n);
  static void deallocate(void *p, size_t n);
#endif  // NDEBUG
};

#else  // GCC_STYLE_ALLOCATOR

template<class Type>
class dallocator : public allocator<Type> {
public:
#ifndef NDEBUG
  INLINE pointer allocate(size_type n, allocator<void>::const_pointer hint = 0);
  //  INLINE void deallocate(pointer p, size_type n);
  INLINE void deallocate(void *p, size_type n);
#endif  // NDEBUG
};
#endif  // GCC_STYLE_ALLOCATOR

#include "dallocator.T"

#endif

