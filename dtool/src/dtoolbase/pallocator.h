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

#if defined(NO_STYLE_ALLOCATOR)
// If we're not trying to make custom allocators (either we don't know
// what kind of syntax this STL library wants, or we're compiling with
// OPTIMIZE 4), then simply use the standard allocator.
#define pallocator allocator

#elif defined(OLD_STYLE_ALLOCATOR)
// Early versions of gcc wanted to use their own kind of allocator,
// somewhat different from the STL standard.  Irix uses this one too.
// It might be inherited from an early draft of the STL standard.

template<class Type>
class pallocator : public alloc {
public:
  INLINE static Type *allocate(size_t n);
  INLINE static void deallocate(void *p, size_t n);
};

#elif defined(GNU_STYLE_ALLOCATOR)
// Later versions of gcc want to use a still different,
// not-quite-standard definition.  Sheesh.

template<class Type>
class pallocator : public allocator<Type> {
public:
  INLINE pallocator();
  template<class _Tp1>
  INLINE pallocator(const pallocator<_Tp1> &other);

  INLINE Type *allocate(size_t n);
  INLINE void deallocate(void *p, size_t n);

  template <class _Tp1> struct rebind {
    typedef pallocator<_Tp1> other;
  };
};

#elif defined(VC6_STYLE_ALLOCATOR)

// The VC6-era definition.
template<class Type>
class pallocator : public allocator<Type> {
public:
  INLINE pointer allocate(size_type n, allocator<void>::const_pointer hint = 0);
  INLINE void deallocate(void *p, size_type n);
};

#elif defined(MODERN_STYLE_ALLOCATOR)

// The final specification?
template<class Type>
class pallocator : public allocator<Type> {
public:
  INLINE pallocator() throw();

  // template member functions in VC++ can only be defined in-class.
  template<class U>
  INLINE pallocator(const pallocator<U> &) throw() { }

  INLINE pointer allocate(size_type n, allocator<void>::const_pointer hint = 0);
  INLINE void deallocate(void *p, size_type n);

  template<class U> struct rebind { 
	  typedef pallocator<U> other;
  };
};

#else
#error Unrecognized allocator symbol defined!
#endif  // *_STYLE_ALLOCATOR

#ifndef NO_STYLE_ALLOCATOR
#include "pallocator.T"
#endif

#endif

