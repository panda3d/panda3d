// Filename: pallocator.T
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

#ifdef GCC_STYLE_ALLOCATOR

#ifndef NDEBUG
template<class Type>
INLINE void *pallocator<Type>::
allocate(size_t n) {
  return (*global_operator_new)(n);
}

template<class Type>
INLINE void pallocator<Type>::
deallocate(void *p, size_t) {
  (*global_operator_delete)(p);
}
#endif  // NDEBUG

#else  // GCC_STYLE_ALLOCATOR

#ifndef NDEBUG
template<class Type>
INLINE pallocator<Type>::pointer pallocator<Type>::
allocate(pallocator<Type>::size_type n, allocator<void>::const_pointer) {
  return (pallocator<Type>::pointer)(*global_operator_new)(n * sizeof(Type));
}

template<class Type>
INLINE void pallocator<Type>::
deallocate(pallocator<Type>::pointer p, allocator<Type>::size_type) {
  (*global_operator_delete)(p);
}
#endif  // NDEBUG

#endif  // GCC_STYLE_ALLOCATOR
