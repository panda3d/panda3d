/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pdeque.h
 * @author drose
 * @date 2001-06-05
 */

#ifndef PDEQUE_H
#define PDEQUE_H

#include "dtoolbase.h"
#include "pallocator.h"
#include "register_type.h"
#include <deque>
#include <initializer_list>

#if !defined(USE_STL_ALLOCATOR) || defined(CPPPARSER)
// If we're not using custom allocators, just use the standard class
// definition.
#define pdeque std::deque

#else

/**
 * This is our own Panda specialization on the default STL deque.  Its main
 * purpose is to call the hooks for MemoryUsage to properly track STL-
 * allocated memory.
 */
template<class Type>
class pdeque : public std::deque<Type, pallocator_array<Type> > {
public:
  typedef pallocator_array<Type> allocator;
  typedef typename std::deque<Type, allocator>::size_type size_type;
  pdeque(TypeHandle type_handle = pdeque_type_handle) : std::deque<Type, pallocator_array<Type> >(allocator(type_handle)) { }
  pdeque(size_type n, TypeHandle type_handle = pdeque_type_handle) : std::deque<Type, pallocator_array<Type> >(n, Type(), allocator(type_handle)) { }
  pdeque(size_type n, const Type &value, TypeHandle type_handle = pdeque_type_handle) : std::deque<Type, pallocator_array<Type> >(n, value, allocator(type_handle)) { }
  pdeque(std::initializer_list<Type> init, TypeHandle type_handle = pdeque_type_handle) : std::deque<Type, pallocator_array<Type> >(std::move(init), allocator(type_handle)) { }
};

#endif  // USE_STL_ALLOCATOR
#endif
