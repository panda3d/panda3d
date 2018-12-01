/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pallocator.T
 * @author drose
 * @date 2001-06-05
 */

template<class Type>
INLINE pallocator_single<Type>::
pallocator_single(TypeHandle type_handle) noexcept :
  _type_handle(type_handle)
{
}

template<class Type>
INLINE Type *pallocator_single<Type>::
allocate(typename pallocator_single<Type>::size_type n, typename std::allocator<void>::const_pointer) {
  TAU_PROFILE("pallocator_single:allocate()", " ", TAU_USER);
  // This doesn't support allocating arrays.
  assert(n == 1);
  return (Type *)ASSUME_ALIGNED(StaticDeletedChain<Type>::allocate(sizeof(Type), _type_handle),
                                MEMORY_HOOK_ALIGNMENT);
}

template<class Type>
INLINE void pallocator_single<Type>::
deallocate(typename pallocator_single<Type>::pointer p, typename pallocator_single<Type>::size_type) {
  TAU_PROFILE("pallocator_single:deallocate()", " ", TAU_USER);
  StaticDeletedChain<Type>::deallocate(p, _type_handle);
}

template<class Type>
INLINE pallocator_array<Type>::
pallocator_array(TypeHandle type_handle) noexcept :
  _type_handle(type_handle)
{
}

template<class Type>
INLINE Type *pallocator_array<Type>::
allocate(typename pallocator_array<Type>::size_type n, typename std::allocator<void>::const_pointer) {
  return (typename pallocator_array<Type>::pointer)
    ASSUME_ALIGNED(_type_handle.allocate_array(n * sizeof(Type)), MEMORY_HOOK_ALIGNMENT);
}

template<class Type>
INLINE void pallocator_array<Type>::
deallocate(typename pallocator_array<Type>::pointer p, typename pallocator_array<Type>::size_type) {
  _type_handle.deallocate_array((void *)p);
}
