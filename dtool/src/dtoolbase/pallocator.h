/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pallocator.h
 * @author drose
 * @date 2001-06-05
 */

#ifndef PALLOCATOR_H
#define PALLOCATOR_H

#include <memory>
#include "dtoolbase.h"
#include "memoryHook.h"
#include "deletedChain.h"
#include "typeHandle.h"

/**
 * This is our own Panda specialization on the default STL allocator.  Its
 * main purpose is to call the hooks for MemoryUsage to properly track STL-
 * allocated memory.
 *
 * pvector, pmap, etc.  are all defined in this directory to use a pallocator.
 *
 * pallocator actually comes it two flavors now: pallocator_single, which can
 * only allocate single instances of an object, and pallocator_array, which
 * can allocate arrays of objects.
 */

#if !defined(USE_STL_ALLOCATOR) || defined(CPPPARSER)
// If we're not trying to make custom allocators (either we don't know what
// kind of syntax this STL library wants, or we're compiling with OPTIMIZE 4),
// then simply use the standard allocator.
#define pallocator_single std::allocator
#define pallocator_array std::allocator

#else

template<class Type>
class pallocator_single : public std::allocator<Type> {
public:
  // Nowadays we cannot implicitly inherit typedefs from base classes in a
  // template class; we must explicitly copy them here.
  typedef typename std::allocator<Type>::pointer pointer;
  typedef typename std::allocator<Type>::reference reference;
  typedef typename std::allocator<Type>::const_pointer const_pointer;
  typedef typename std::allocator<Type>::const_reference const_reference;
  typedef typename std::allocator<Type>::size_type size_type;

  INLINE pallocator_single(TypeHandle type_handle) noexcept;

  // template member functions in VC++ can only be defined in-class.
  template<class U>
  INLINE pallocator_single(const pallocator_single<U> &copy) noexcept :
    _type_handle(copy._type_handle) { }

  INLINE Type *allocate(size_type n, std::allocator<void>::const_pointer hint = 0)
    RETURNS_ALIGNED(MEMORY_HOOK_ALIGNMENT);
  INLINE void deallocate(pointer p, size_type n);

  template<class U> struct rebind {
    typedef pallocator_single<U> other;
  };

  TypeHandle _type_handle;
};

template<class Type>
class pallocator_array : public std::allocator<Type> {
public:
  // Nowadays we cannot implicitly inherit typedefs from base classes in a
  // template class; we must explicitly copy them here.
  typedef typename std::allocator<Type>::pointer pointer;
  typedef typename std::allocator<Type>::reference reference;
  typedef typename std::allocator<Type>::const_pointer const_pointer;
  typedef typename std::allocator<Type>::const_reference const_reference;
  typedef typename std::allocator<Type>::size_type size_type;

  INLINE pallocator_array(TypeHandle type_handle = TypeHandle::none()) noexcept;

  // template member functions in VC++ can only be defined in-class.
  template<class U>
  INLINE pallocator_array(const pallocator_array<U> &copy) noexcept :
    _type_handle(copy._type_handle) { }

  INLINE Type *allocate(size_type n, std::allocator<void>::const_pointer hint = 0)
    RETURNS_ALIGNED(MEMORY_HOOK_ALIGNMENT);
  INLINE void deallocate(pointer p, size_type n);

  template<class U> struct rebind {
    typedef pallocator_array<U> other;
  };

  TypeHandle _type_handle;
};

#include "pallocator.T"

#endif  // USE_STL_ALLOCATOR

#endif
