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

template<class Type>
INLINE pallocator_single<Type>::
pallocator_single() throw() {
}

template<class Type>
INLINE TYPENAME pallocator_single<Type>::pointer pallocator_single<Type>::
allocate(TYPENAME pallocator_single<Type>::size_type n, TYPENAME allocator<void>::const_pointer) {
  TAU_PROFILE("pallocator_single:allocate()", " ", TAU_USER);
  // This doesn't support allocating arrays.
  assert(n == 1);
  return DeletedChain<Type>::allocate(sizeof(Type));
}

template<class Type>
INLINE void pallocator_single<Type>::
deallocate(TYPENAME pallocator_single<Type>::pointer p, TYPENAME pallocator_single<Type>::size_type) {
  TAU_PROFILE("pallocator_single:deallocate()", " ", TAU_USER);
  return DeletedChain<Type>::deallocate(p);
}

template<class Type>
INLINE pallocator_array<Type>::
pallocator_array() throw() {
}

template<class Type>
INLINE TYPENAME pallocator_array<Type>::pointer pallocator_array<Type>::
allocate(TYPENAME pallocator_array<Type>::size_type n, TYPENAME allocator<void>::const_pointer) {
  TAU_PROFILE("pallocator_array:allocate()", " ", TAU_USER);
#ifdef DO_MEMORY_USAGE
  return (TYPENAME pallocator_array<Type>::pointer)(*global_operator_new)(n * sizeof(Type));
#else
  return (TYPENAME pallocator_array<Type>::pointer)malloc(n * sizeof(Type));
#endif  // DO_MEMORY_USAGE
}

template<class Type>
INLINE void pallocator_array<Type>::
deallocate(TYPENAME pallocator_array<Type>::pointer p, TYPENAME pallocator_array<Type>::size_type) {
  TAU_PROFILE("pallocator_array:deallocate()", " ", TAU_USER);
#ifdef DO_MEMORY_USAGE
  (*global_operator_delete)(p);
#else
  free(p);
#endif  // DO_MEMORY_USAGE
}
