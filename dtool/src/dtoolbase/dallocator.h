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

#if defined(NO_STYLE_ALLOCATOR)
// If we're not trying to make custom allocators (either we don't know
// what kind of syntax this STL library wants, or we're compiling with
// OPTIMIZE 4), then simply use the standard allocator.
#define dallocator allocator

#elif defined(OLD_STYLE_ALLOCATOR)
// Early versions of gcc wanted to use their own kind of allocator,
// somewhat different from the STL standard.  Irix uses this one too.
// It might be inherited from an early draft of the STL standard.

template<class Type>
class dallocator : public alloc {
public:
  INLINE static Type *allocate(size_t n);
  INLINE static void deallocate(void *p, size_t n);
};

#elif defined(GNU_STYLE_ALLOCATOR)
// Later versions of gcc want to use a still different,
// not-quite-standard definition.  Sheesh.

template<class Type>
class dallocator : public allocator<Type> {
public:
  INLINE dallocator();
  template<class _Tp1>
  INLINE dallocator(const dallocator<_Tp1> &other);

  INLINE Type *allocate(size_t n);
  INLINE void deallocate(void *p, size_t n);

  template <class _Tp1> struct rebind {
    typedef dallocator<_Tp1> other;
  };
};

#elif defined(MODERN_STYLE_ALLOCATOR)

// The final specification?
template<class Type>
class dallocator : public allocator<Type> {
public:
  // There seems to be a bug in VC++ 2003 that requires these typedefs
  // to be made explicitly.
  typedef TYPENAME allocator<Type>::pointer pointer;
  typedef TYPENAME allocator<Type>::reference reference;
  typedef TYPENAME allocator<Type>::const_pointer const_pointer;
  typedef TYPENAME allocator<Type>::const_reference const_reference;

  INLINE dallocator() throw();

  // template member functions in VC++ can only be defined in-class.
  template<class U>
  INLINE dallocator(const dallocator<U> &copy) throw() { }

  INLINE pointer allocate(size_type n, allocator<void>::const_pointer hint = 0);
  INLINE void deallocate(void *p, size_type n);

  template<class U> struct rebind { 
    typedef dallocator<U> other; 
  };
};

#else
#error Unrecognized allocator symbol defined!
#endif  // *_STYLE_ALLOCATOR

#ifndef NO_STYLE_ALLOCATOR
#include "dallocator.T"
#endif

#endif

