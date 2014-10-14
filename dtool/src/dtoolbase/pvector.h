// Filename: pvector.h
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

#ifndef PVECTOR_H
#define PVECTOR_H

#include <vector>

#include "dtoolbase.h"
#include "pallocator.h"
#include "register_type.h"

#ifndef USE_STL_ALLOCATOR
// If we're not using custom allocators, just use the standard class
// definition.
#define pvector vector

#else

////////////////////////////////////////////////////////////////////
//       Class : pvector
// Description : This is our own Panda specialization on the default
//               STL vector.  Its main purpose is to call the hooks
//               for MemoryUsage to properly track STL-allocated
//               memory.
////////////////////////////////////////////////////////////////////
template<class Type>
class pvector : public vector<Type, pallocator_array<Type> > {
public:
  typedef pallocator_array<Type> allocator;
  typedef vector<Type, allocator> base_class;
  typedef TYPENAME base_class::size_type size_type;

  pvector(TypeHandle type_handle = pvector_type_handle) : base_class(allocator(type_handle)) { }
  pvector(const pvector<Type> &copy) : base_class(copy) { }
  pvector(size_type n, TypeHandle type_handle = pvector_type_handle) : base_class(n, Type(), allocator(type_handle)) { }
  pvector(size_type n, const Type &value, TypeHandle type_handle = pvector_type_handle) : base_class(n, value, allocator(type_handle)) { }
  pvector(const Type *begin, const Type *end, TypeHandle type_handle = pvector_type_handle) : base_class(begin, end, allocator(type_handle)) { }
};

#endif  // USE_STL_ALLOCATOR

#endif
