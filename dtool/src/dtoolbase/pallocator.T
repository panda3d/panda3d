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

#if defined(OLD_STYLE_ALLOCATOR)

template<class Type>
INLINE Type *pallocator<Type>::
allocate(size_t n) {
  return (Type *)(*global_operator_new)(n);
}

template<class Type>
INLINE void pallocator<Type>::
deallocate(void *p, size_t) {
  (*global_operator_delete)(p);
}

#elif defined(GNU_STYLE_ALLOCATOR)

template<class Type>
INLINE pallocator<Type>::
pallocator() {
}

template<class Type>
template<class _Tp1>
INLINE pallocator<Type>::
pallocator(const pallocator<_Tp1> &) {
}

template<class Type>
INLINE Type *pallocator<Type>::
allocate(size_t n) {
  return (Type *)(*global_operator_new)(n * sizeof(Type));
}

template<class Type>
INLINE void pallocator<Type>::
deallocate(void *p, size_t) {
  (*global_operator_delete)(p);
}

#elif VC6_STYLE_ALLOCATOR

template<class Type>
INLINE pallocator<Type>::pointer pallocator<Type>::
allocate(pallocator<Type>::size_type n, allocator<void>::const_pointer) {
  return (pallocator<Type>::pointer)(*global_operator_new)(n * sizeof(Type));
}

template<class Type>
INLINE void pallocator<Type>::
//deallocate(pallocator<Type>::pointer p, allocator<Type>::size_type) {
deallocate(void *p, allocator<Type>::size_type) {
  (*global_operator_delete)(p);
}

#elif MODERN_STYLE_ALLOCATOR

template<class Type>
INLINE pallocator<Type>::
pallocator() throw() {
}

template<class Type>
INLINE TYPENAME pallocator<Type>::pointer pallocator<Type>::
allocate(TYPENAME pallocator<Type>::size_type n, TYPENAME allocator<void>::const_pointer) {
  return (TYPENAME pallocator<Type>::pointer)(*global_operator_new)(n * sizeof(Type));
}

template<class Type>
INLINE void pallocator<Type>::
deallocate(TYPENAME pallocator<Type>::pointer p, TYPENAME pallocator<Type>::size_type) {
  (*global_operator_delete)(p);
}

#endif  // *_STYLE_ALLOCATOR
