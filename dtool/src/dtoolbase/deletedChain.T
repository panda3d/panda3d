// Filename: deletedChain.T
// Created by:  drose (01Apr06)
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
  assert(size <= sizeof(Type));

  size_t alloc_size = sizeof(Type);
#ifdef USE_DELETEDCHAINFLAG
  // In development mode, we also need to reserve space for _flag.
  alloc_size += sizeof(PN_int32);
#endif  // NDEBUG

  ObjectNode *obj;

  init_lock();
  _lock->lock();
  if (_deleted_chain != (ObjectNode *)NULL) {
    obj = _deleted_chain;
    _deleted_chain = _deleted_chain->_next;
    _lock->release();

#ifdef USE_DELETEDCHAINFLAG
    assert(obj->_flag == (PN_int32)DCF_deleted);
    obj->_flag = DCF_alive;
#endif  // NDEBUG

    Type *ptr = node_to_type(obj);

#ifdef DO_MEMORY_USAGE
    type_handle.dec_memory_usage(TypeHandle::MC_deleted_chain_inactive, alloc_size);
    type_handle.inc_memory_usage(TypeHandle::MC_deleted_chain_active, alloc_size);
    memory_hook->mark_pointer(ptr, max(size, sizeof(ObjectNode)), make_ref_ptr(ptr));
#endif  // DO_MEMORY_USAGE

    return ptr;
  }
  _lock->release();

  // If we get here, the deleted_chain is empty; we have to allocate a
  // new object from the system pool.

  alloc_size = max(alloc_size, sizeof(ObjectNode));
  obj = (ObjectNode *)NeverFreeMemory::alloc(alloc_size);

#ifdef USE_DELETEDCHAINFLAG
  obj->_flag = DCF_alive;
#endif  // USE_DELETEDCHAINFLAG

  Type *ptr = node_to_type(obj);

#ifdef DO_MEMORY_USAGE
  type_handle.inc_memory_usage(TypeHandle::MC_deleted_chain_active, alloc_size);
  memory_hook->mark_pointer(ptr, max(size, sizeof(ObjectNode)), make_ref_ptr(ptr));
#endif  // DO_MEMORY_USAGE

  return ptr;
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
  assert(ptr != (Type *)NULL);

#ifdef DO_MEMORY_USAGE
  size_t alloc_size = sizeof(Type);

  memory_hook->mark_pointer(ptr, 0, make_ref_ptr(ptr));

#ifdef USE_DELETEDCHAINFLAG
  // In development mode, we also need to reserve space for _flag.
  alloc_size += sizeof(PN_int32);
#endif  // NDEBUG
  type_handle.dec_memory_usage(TypeHandle::MC_deleted_chain_active, alloc_size);
  type_handle.inc_memory_usage(TypeHandle::MC_deleted_chain_inactive, alloc_size);

#endif  // DO_MEMORY_USAGE

  ObjectNode *obj = type_to_node(ptr);

#ifdef USE_DELETEDCHAINFLAG
  PN_int32 orig_flag = AtomicAdjust::compare_and_exchange(obj->_flag, DCF_alive, DCF_deleted);

  // If this assertion is triggered, you double-deleted an object.
  assert(orig_flag != (PN_int32)DCF_deleted);

  // If this assertion is triggered, you tried to delete an object
  // that was never allocated, or you have heap corruption.
  assert(orig_flag == (PN_int32)DCF_alive);
#endif  // NDEBUG

  // We have to test the lock again, even though we must have
  // allocated this object at some point, because Windows might make
  // multiple different DeletedChain instances for a particular Type,
  // and there's no guarantee this one is the same one we used to
  // allocate this object.
  init_lock();
  _lock->lock();

  obj->_next = _deleted_chain;
  _deleted_chain = obj;

  _lock->release();
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
  if (ptr == (Type *)NULL) {
    return false;
  }

#ifdef USE_DELETEDCHAINFLAG
  const ObjectNode *obj = type_to_node((Type *)ptr);
  return AtomicAdjust::get(obj->_flag) == DCF_alive;
#else
  return true;
#endif  // NDEBUG
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
//     Function: DeletedChain::node_to_type
//       Access: Private, Static
//  Description: Casts an ObjectNode* to a Type*.
////////////////////////////////////////////////////////////////////
template<class Type>
INLINE Type *DeletedChain<Type>::
node_to_type(TYPENAME DeletedChain<Type>::ObjectNode *node) {
#ifdef USE_DELETEDCHAINFLAG
  // In development mode, we increment the pointer so that the
  // returned data does not overlap our _flags member.
  return (Type *)(((PN_int32 *)node) + 1);
#else
  return (Type *)node;
#endif  // NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function: DeletedChain::type_to_node
//       Access: Private, Static
//  Description: Casts a Type* to an ObjectNode* .
////////////////////////////////////////////////////////////////////
template<class Type>
INLINE TYPENAME DeletedChain<Type>::ObjectNode *DeletedChain<Type>::
type_to_node(Type *ptr) {
#ifdef USE_DELETEDCHAINFLAG
  // In development mode, we decrement the pointer to undo the
  // increment we did above.
  return (ObjectNode *)(((PN_int32 *)ptr) - 1);
#else
  return (ObjectNode *)ptr;
#endif  // NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function: DeletedChain::init_lock
//       Access: Private
//  Description: Ensures the lock pointer has been allocated.
////////////////////////////////////////////////////////////////////
template<class Type>
INLINE void DeletedChain<Type>::
init_lock() {
  if (_lock == (MutexImpl *)NULL) {
    do_init_lock(_lock);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DeletedChain::do_init_lock
//       Access: Private
//  Description: Allocates the lock pointer if necessary.  Takes some
//               pains to protect itself from race conditions.
//
//               We have to receive the MutexImpl object as a
//               parameter, because this is a non-inline function, and
//               the template pointer might get evaluated differently
//               for inline vs. non-inline functions.
////////////////////////////////////////////////////////////////////
template<class Type>
void DeletedChain<Type>::
do_init_lock(MutexImpl *&lock) {
  MutexImpl *new_lock = new MutexImpl;

  MutexImpl *result;
  result = (MutexImpl *)AtomicAdjust::compare_and_exchange_ptr((void * TVOLATILE &)lock, (void *)NULL, (void *)new_lock);
  
  if (result != NULL) {
    delete new_lock;
  }
  
  assert(lock != (MutexImpl *)NULL);
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
