// Filename: dallocator.T
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

#if defined(OLD_STYLE_ALLOCATOR)

template<class Type>
INLINE Type *dallocator<Type>::
allocate(size_t n) {
  return (Type *)default_operator_new(n);
}

template<class Type>
INLINE void dallocator<Type>::
deallocate(void *p, size_t) {
  default_operator_delete(p);
}

#elif defined(GNU_STYLE_ALLOCATOR)

template<class Type>
INLINE dallocator<Type>::
dallocator() {
}

template<class Type>
template<class _Tp1>
INLINE dallocator<Type>::
dallocator(const dallocator<_Tp1> &) {
}

template<class Type>
INLINE Type *dallocator<Type>::
allocate(size_t n) {
  return (Type *)default_operator_new(n * sizeof(Type));
}

template<class Type>
INLINE void dallocator<Type>::
deallocate(void *p, size_t) {
  default_operator_delete(p);
}

#elif MODERN_STYLE_ALLOCATOR

template<class Type>
INLINE dallocator<Type>::
dallocator() throw() {
}

template<class Type>
INLINE dallocator<Type>::pointer dallocator<Type>::
allocate(dallocator<Type>::size_type n, allocator<void>::const_pointer) {
  return (dallocator<Type>::pointer)(*global_operator_new)(n * sizeof(Type));
}

template<class Type>
INLINE void dallocator<Type>::
//deallocate(dallocator<Type>::pointer p, allocator<Type>::size_type) {
deallocate(void *p, allocator<Type>::size_type) {
  (*global_operator_delete)(p);
}

#endif  // *_STYLE_ALLOCATOR
