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
pallocator_single(TypeHandle type_handle) NOEXCEPT :
  _type_handle(type_handle)
{
}

template<class Type>
INLINE TYPENAME pallocator_single<Type>::pointer pallocator_single<Type>::
allocate(TYPENAME pallocator_single<Type>::size_type n, TYPENAME allocator<void>::const_pointer) {
  TAU_PROFILE("pallocator_single:allocate()", " ", TAU_USER);
  // This doesn't support allocating arrays.
  assert(n == 1);
  return (Type *)ASSUME_ALIGNED(StaticDeletedChain<Type>::allocate(sizeof(Type), _type_handle),
                                MEMORY_HOOK_ALIGNMENT);
}

template<class Type>
INLINE void pallocator_single<Type>::
deallocate(TYPENAME pallocator_single<Type>::pointer p, TYPENAME pallocator_single<Type>::size_type) {
  TAU_PROFILE("pallocator_single:deallocate()", " ", TAU_USER);
  StaticDeletedChain<Type>::deallocate(p, _type_handle);
}

template<class Type>
INLINE pallocator_array<Type>::
pallocator_array(TypeHandle type_handle) NOEXCEPT :
  _type_handle(type_handle)
{
}

template<class Type>
INLINE TYPENAME pallocator_array<Type>::pointer pallocator_array<Type>::
allocate(TYPENAME pallocator_array<Type>::size_type n, TYPENAME allocator<void>::const_pointer) {
  TAU_PROFILE("pallocator_array:allocate()", " ", TAU_USER);
#ifdef DO_MEMORY_USAGE
  size_t alloc_size = n * sizeof(Type);
  void *ptr = (TYPENAME pallocator_array<Type>::pointer)PANDA_MALLOC_ARRAY(alloc_size);
#ifdef _DEBUG
  assert(alloc_size == MemoryHook::get_ptr_size(ptr));
#endif
  _type_handle.inc_memory_usage(TypeHandle::MC_array, alloc_size);
  return (TYPENAME pallocator_array<Type>::pointer)ASSUME_ALIGNED(ptr, MEMORY_HOOK_ALIGNMENT);
#else
  return (TYPENAME pallocator_array<Type>::pointer)PANDA_MALLOC_ARRAY(n * sizeof(Type));
#endif  // DO_MEMORY_USAGE
}

template<class Type>
INLINE void pallocator_array<Type>::
deallocate(TYPENAME pallocator_array<Type>::pointer p, TYPENAME pallocator_array<Type>::size_type) {
  TAU_PROFILE("pallocator_array:deallocate()", " ", TAU_USER);
#ifdef DO_MEMORY_USAGE
  // Now we need to recover the total number of bytes.  Fortunately, in the
  // case of DO_MEMORY_USAGE, MemoryHook already keeps track of this.
  void *ptr = (void *)p;
  size_t alloc_size = MemoryHook::get_ptr_size(ptr);
  _type_handle.dec_memory_usage(TypeHandle::MC_array, alloc_size);
  PANDA_FREE_ARRAY(ptr);
#else
  PANDA_FREE_ARRAY(p);
#endif  // DO_MEMORY_USAGE
}
