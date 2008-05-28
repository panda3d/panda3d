// Filename: pdeque.h
// Created by:  drose (05Jun01)
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

#ifndef PDEQUE_H
#define PDEQUE_H

#include "dtoolbase.h"
#include "pallocator.h"
#include "register_type.h"
#include <deque>

#ifndef USE_STL_ALLOCATOR
// If we're not using custom allocators, just use the standard class
// definition.
#define pdeque deque 

#else

////////////////////////////////////////////////////////////////////
//       Class : pdeque
// Description : This is our own Panda specialization on the default
//               STL deque.  Its main purpose is to call the hooks
//               for MemoryUsage to properly track STL-allocated
//               memory.
////////////////////////////////////////////////////////////////////
template<class Type>
class pdeque : public deque<Type, pallocator_array<Type> > {
public:
  typedef pallocator_array<Type> allocator;
  typedef TYPENAME deque<Type, allocator>::size_type size_type;
  pdeque(TypeHandle type_handle = pdeque_type_handle) : deque<Type, pallocator_array<Type> >(allocator(type_handle)) { }
  pdeque(const pdeque<Type> &copy) : deque<Type, pallocator_array<Type> >(copy) { }
  pdeque(size_type n, TypeHandle type_handle = pdeque_type_handle) : deque<Type, pallocator_array<Type> >(n, Type(), allocator(type_handle)) { }
  pdeque(size_type n, const Type &value, TypeHandle type_handle = pdeque_type_handle) : deque<Type, pallocator_array<Type> >(n, value, allocator(type_handle)) { }
};

#endif  // USE_STL_ALLOCATOR
#endif
