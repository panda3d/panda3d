// Filename: dallocator.h
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

#ifndef DALLOCATOR_H
#define DALLOCATOR_H
#include <memory>

#include "dtoolbase.h"

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

#ifndef USE_STL_ALLOCATOR
// If we're not trying to make custom allocators (either we don't know
// what kind of syntax this STL library wants, or we're compiling with
// OPTIMIZE 4), then simply use the standard allocator.
#define dallocator allocator

#else

template<class Type>
class dallocator : public allocator<Type> {
public:
  // There seems to be a bug in VC++ 2003 that requires these typedefs
  // to be made explicitly.
  typedef TYPENAME allocator<Type>::pointer pointer;
  typedef TYPENAME allocator<Type>::reference reference;
  typedef TYPENAME allocator<Type>::const_pointer const_pointer;
  typedef TYPENAME allocator<Type>::const_reference const_reference;
  typedef TYPENAME allocator<Type>::size_type size_type;

  INLINE dallocator() throw();

  // template member functions in VC++ can only be defined in-class.
  template<class U>
  INLINE dallocator(const dallocator<U> &copy) throw() { }

  INLINE pointer allocate(size_type n, allocator<void>::const_pointer hint = 0);
  INLINE void deallocate(pointer p, size_type n);

  template<class U> struct rebind { 
    typedef dallocator<U> other; 
  };
};

#include "dallocator.T"

#endif  // USE_STL_ALLOCATOR

#endif

