// Filename: pdeque.h
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

#ifndef PDEQUE_H
#define PDEQUE_H

#include "dtoolbase.h"
#include "pallocator.h"
#include <deque>

#ifdef NO_STYLE_ALLOCATOR
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
class pdeque : public deque<Type, pallocator<Type> > {
public:
  typedef TYPENAME deque<Type, pallocator<Type> >::size_type size_type;
  pdeque() : deque<Type, pallocator<Type> >() { }
  pdeque(const pdeque<Type> &copy) : deque<Type, pallocator<Type> >(copy) { }
  pdeque(size_type n) : deque<Type, pallocator<Type> >(n) { }
  pdeque(size_type n, const Type &value) : deque<Type, pallocator<Type> >(n, value) { }
};

#endif  // NO_STYLE_ALLOCATOR
#endif
