// Filename: pallocator.h
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

#ifndef PALLOCATOR_H
#define PALLOCATOR_H

#include <memory>
#include "dtoolbase.h"
#include "deletedChain.h"

////////////////////////////////////////////////////////////////////
//       Class : pallocator
// Description : This is our own Panda specialization on the default
//               STL allocator.  Its main purpose is to call the hooks
//               for MemoryUsage to properly track STL-allocated
//               memory.
//
//               pvector, pmap, etc. are all defined in this directory
//               to use a pallocator.
//
//               pallocator actually comes it two flavors now:
//               pallocator_single, which can only allocate single
//               instances of an object, and pallocator_array, which
//               can allocate arrays of objects.
////////////////////////////////////////////////////////////////////

#ifndef USE_STL_ALLOCATOR
// If we're not trying to make custom allocators (either we don't know
// what kind of syntax this STL library wants, or we're compiling with
// OPTIMIZE 4), then simply use the standard allocator.
#define pallocator_single allocator
#define pallocator_array allocator

#else

template<class Type>
class pallocator_single : public allocator<Type> {
public:
  // Nowadays we cannot implicitly inherit typedefs from base classes
  // in a template class; we must explicitly copy them here.
  typedef TYPENAME allocator<Type>::pointer pointer;
  typedef TYPENAME allocator<Type>::reference reference;
  typedef TYPENAME allocator<Type>::const_pointer const_pointer;
  typedef TYPENAME allocator<Type>::const_reference const_reference;
  typedef TYPENAME allocator<Type>::size_type size_type;

  INLINE pallocator_single() throw();

  // template member functions in VC++ can only be defined in-class.
  template<class U>
  INLINE pallocator_single(const pallocator_single<U> &) throw() { }

  INLINE pointer allocate(size_type n, allocator<void>::const_pointer hint = 0);
  INLINE void deallocate(pointer p, size_type n);

  template<class U> struct rebind { 
    typedef pallocator_single<U> other;
  };
};

template<class Type>
class pallocator_array : public allocator<Type> {
public:
  // Nowadays we cannot implicitly inherit typedefs from base classes
  // in a template class; we must explicitly copy them here.
  typedef TYPENAME allocator<Type>::pointer pointer;
  typedef TYPENAME allocator<Type>::reference reference;
  typedef TYPENAME allocator<Type>::const_pointer const_pointer;
  typedef TYPENAME allocator<Type>::const_reference const_reference;
  typedef TYPENAME allocator<Type>::size_type size_type;

  INLINE pallocator_array() throw();

  // template member functions in VC++ can only be defined in-class.
  template<class U>
  INLINE pallocator_array(const pallocator_array<U> &) throw() { }

  INLINE pointer allocate(size_type n, allocator<void>::const_pointer hint = 0);
  INLINE void deallocate(pointer p, size_type n);

  template<class U> struct rebind { 
    typedef pallocator_array<U> other;
  };
};

#include "pallocator.T"

#endif  // USE_STL_ALLOCATOR

#endif

