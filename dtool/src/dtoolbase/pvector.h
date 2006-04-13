// Filename: pvector.h
// Created by:  drose (05Jun01)
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

#ifndef PVECTOR_H
#define PVECTOR_H

#include <vector>

#include "dtoolbase.h"
#include "pallocator.h"

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
  typedef vector<Type, pallocator_array<Type> > base_class;
  typedef TYPENAME base_class::size_type size_type;

  pvector() : base_class() { }
  pvector(const pvector<Type> &copy) : base_class(copy) { }
  pvector(size_type n) : base_class(n) { }
  pvector(size_type n, const Type &value) : base_class(n, value) { }
  pvector(const Type *begin, const Type *end) : base_class(begin, end) { }
};

#endif  // USE_STL_ALLOCATOR
#endif

