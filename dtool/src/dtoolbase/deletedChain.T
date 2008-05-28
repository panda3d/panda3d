// Filename: deletedChain.T
// Created by:  drose (01Apr06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

template<class Type>
DeletedChain<Type> StaticDeletedChain<Type>::_chain;

////////////////////////////////////////////////////////////////////
//     Function: DeletedChain::allocate
//       Access: Public
//  Description: Allocates the memory for a new object of Type.
////////////////////////////////////////////////////////////////////
template<class Type>
INLINE Type *DeletedChain<Type>::
allocate(size_t size, TypeHandle type_handle) {
  TAU_PROFILE("Type *DeletedChain<Type>::allocate(size_t, TypeHandle)", " ", TAU_USER);
  init_deleted_chain();
  void *ptr = _chain->allocate(size, type_handle);

#ifdef DO_MEMORY_USAGE
  memory_hook->mark_pointer(ptr, _chain->get_buffer_size(), make_ref_ptr(ptr));
#endif  // DO_MEMORY_USAGE

  return (Type *)ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: DeletedChain::deallocate
//       Access: Public
//  Description: Frees the memory for an object of Type.
////////////////////////////////////////////////////////////////////
template<class Type>
INLINE void DeletedChain<Type>::
deallocate(Type *ptr, TypeHandle type_handle) {
  TAU_PROFILE("void DeletedChain<Type>::deallocate(Type *, TypeHandle)", " ", TAU_USER);

#ifdef DO_MEMORY_USAGE
  memory_hook->mark_pointer(ptr, 0, make_ref_ptr(ptr));
#endif

  // We have to call this again, even though it must have been called
  // in allocate(), because with template resolution across dll's it
  // is possible that this DeletedChain object is not the same one
  // that allocated the object.
  init_deleted_chain();

  _chain->deallocate(ptr, type_handle);
}

////////////////////////////////////////////////////////////////////
//     Function: DeletedChain::validate
//       Access: Public
//  Description: Returns true if the pointer is valid, false if it has
//               been deleted or if it was never a valid pointer.
//
//               This is only meaningful in debug mode, where
//               USE_DELETEDCHAINFLAG is defined.  If not, this
//               trivially returns true.
////////////////////////////////////////////////////////////////////
template<class Type>
INLINE bool DeletedChain<Type>::
validate(const Type *ptr) {
  TAU_PROFILE("bool DeletedChain<Type>::validate(Type *)", " ", TAU_USER);

#ifdef USE_DELETEDCHAINFLAG
  init_deleted_chain();
  return _chain->validate((void *)ptr);
#else
  return true;
#endif  // USE_DELETEDCHAINFLAG
}

////////////////////////////////////////////////////////////////////
//     Function: DeletedChain::make_ref_ptr
//       Access: Public, Static
//  Description: This method has two overloads: one that accepts a
//               void *, and one that accepts a ReferenceCount *.  We
//               rely on the C++ compiler to select the most
//               appropriate one for a given type to return the
//               ReferenceCount pointer that corresponds to a
//               particular type, or NULL if the type does not inherit
//               from ReferenceCount.
////////////////////////////////////////////////////////////////////
template<class Type>
INLINE ReferenceCount *DeletedChain<Type>::
make_ref_ptr(void *) {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DeletedChain::make_ref_ptr
//       Access: Public, Static
//  Description: This method has two overloads: one that accepts a
//               void *, and one that accepts a ReferenceCount *.  We
//               rely on the C++ compiler to select the most
//               appropriate one for a given type to return the
//               ReferenceCount pointer that corresponds to a
//               particular type, or NULL if the type does not inherit
//               from ReferenceCount.
////////////////////////////////////////////////////////////////////
template<class Type>
INLINE ReferenceCount *DeletedChain<Type>::
make_ref_ptr(ReferenceCount *ptr) {
  return ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: DeletedChain::init_deleted_chain
//       Access: Private
//  Description: Assigns the _chain pointer if it is not already
//               assigned.  This can't be done by a constructor, since
//               often the DeletedChain instance is used before its
//               static construct has had a chance to be called.
////////////////////////////////////////////////////////////////////
template<class Type>
void DeletedChain<Type>::
init_deleted_chain() {
  if (_chain == (DeletedBufferChain *)NULL) {
    init_memory_hook();
    _chain = memory_hook->get_deleted_chain(sizeof(Type));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: StaticDeletedChain::allocate
//       Access: Public, Static
//  Description: Allocates the memory for a new object of Type.
////////////////////////////////////////////////////////////////////
template<class Type>
INLINE Type *StaticDeletedChain<Type>::
allocate(size_t size, TypeHandle type_handle) {
  return _chain.allocate(size, type_handle);
}

////////////////////////////////////////////////////////////////////
//     Function: StaticDeletedChain::deallocate
//       Access: Public
//  Description: Frees the memory for an object of Type.
////////////////////////////////////////////////////////////////////
template<class Type>
INLINE void StaticDeletedChain<Type>::
deallocate(Type *ptr, TypeHandle type_handle) {
  _chain.deallocate(ptr, type_handle);
}

////////////////////////////////////////////////////////////////////
//     Function: StaticDeletedChain::validate
//       Access: Public
//  Description: Returns true if the pointer is valid, false if it has
//               been deleted or if it was never a valid pointer.
//
//               This is only meaningful in debug mode, where
//               USE_DELETEDCHAINFLAG is defined.  If not, this
//               trivially returns true.
////////////////////////////////////////////////////////////////////
template<class Type>
INLINE bool StaticDeletedChain<Type>::
validate(const Type *ptr) {
  return _chain.validate(ptr);
}
