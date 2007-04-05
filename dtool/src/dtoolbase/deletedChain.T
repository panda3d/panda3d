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
TVOLATILE TYPENAME DeletedChain<Type>::ObjectNode * TVOLATILE DeletedChain<Type>::_deleted_chain;

#ifndef DELETED_CHAIN_USE_ATOMIC_EXCHANGE
template<class Type>
MutexImpl *DeletedChain<Type>::_lock;
#endif

////////////////////////////////////////////////////////////////////
//     Function: DeletedChain::allocate
//       Access: Public, Static
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

  TVOLATILE ObjectNode *obj;

#ifndef DELETED_CHAIN_USE_ATOMIC_EXCHANGE
  init_lock();
  _lock->lock();
  if (_deleted_chain != (ObjectNode *)NULL) {
#ifdef DO_MEMORY_USAGE
    type_handle.dec_memory_usage(TypeHandle::MC_deleted_chain_inactive, alloc_size);
    type_handle.inc_memory_usage(TypeHandle::MC_deleted_chain_active, alloc_size);
#endif  // DO_MEMORY_USAGE

    obj = _deleted_chain;
    _deleted_chain = _deleted_chain->_next;
    _lock->release();
#ifdef USE_DELETEDCHAINFLAG
    assert(obj->_flag == (PN_int32)DCF_deleted);
    obj->_flag = DCF_alive;
#endif  // NDEBUG
    return node_to_type(obj);
  }
  _lock->release();

#else  // DELETED_CHAIN_USE_ATOMIC_EXCHANGE
  obj = _deleted_chain;
  while (obj != (ObjectNode *)NULL) {
    TVOLATILE ObjectNode *next = obj->_next;
    TVOLATILE ObjectNode *result = (ObjectNode *)AtomicAdjust::compare_and_exchange_ptr((void * TVOLATILE &)_deleted_chain, (void *)obj, (void *)next);
    if (result == obj) {
      // We got it.
#ifdef DO_MEMORY_USAGE
      type_handle.dec_memory_usage(TypeHandle::MC_deleted_chain_inactive, alloc_size);
      type_handle.inc_memory_usage(TypeHandle::MC_deleted_chain_active, alloc_size);
#endif  // DO_MEMORY_USAGE
#ifdef USE_DELETEDCHAINFLAG
      PN_int32 orig_flag = AtomicAdjust::compare_and_exchange(obj->_flag, DCF_deleted, DCF_alive);
      assert(orig_flag == (PN_int32)DCF_deleted);
#endif  // NDEBUG
      return node_to_type(obj);
    }
    // Someone else grabbed the top link first.  Try again.
    obj = _deleted_chain;
  }

#endif  // DELETED_CHAIN_USE_ATOMIC_EXCHANGE

  // If we get here, the deleted_chain is empty; we have to allocate a
  // new object from the system pool.

#ifdef DO_MEMORY_USAGE
  type_handle.inc_memory_usage(TypeHandle::MC_deleted_chain_active, alloc_size);
#endif  // DO_MEMORY_USAGE
  size = max(alloc_size, sizeof(ObjectNode));

#ifdef DO_MEMORY_USAGE
  obj = (ObjectNode *)(*global_operator_new)(alloc_size);
#else
  obj = (ObjectNode *)malloc(alloc_size);
#endif
#ifdef USE_DELETEDCHAINFLAG
  obj->_flag = DCF_alive;
#endif  // NDEBUG

  return node_to_type(obj);
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
#ifdef USE_DELETEDCHAINFLAG
  // In development mode, we also need to reserve space for _flag.
  alloc_size += sizeof(PN_int32);
#endif  // NDEBUG
  type_handle.dec_memory_usage(TypeHandle::MC_deleted_chain_active, alloc_size);
  type_handle.inc_memory_usage(TypeHandle::MC_deleted_chain_inactive, alloc_size);
#endif  // DO_MEMORY_USAGE

  TVOLATILE ObjectNode *obj = type_to_node(ptr);

#ifdef USE_DELETEDCHAINFLAG
  PN_int32 orig_flag = AtomicAdjust::compare_and_exchange(obj->_flag, DCF_alive, DCF_deleted);

  // If this assertion is triggered, you double-deleted an object.
  assert(orig_flag != (PN_int32)DCF_deleted);

  // If this assertion is triggered, you tried to delete an object
  // that was never allocated, or you have heap corruption.
  assert(orig_flag == (PN_int32)DCF_alive);
#endif  // NDEBUG

#ifndef DELETED_CHAIN_USE_ATOMIC_EXCHANGE
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

#else  // DELETED_CHAIN_USE_ATOMIC_EXCHANGE

  TVOLATILE ObjectNode *result;
  TVOLATILE ObjectNode *next;

  do {
    next = _deleted_chain;
    obj->_next = next;
    result = type_to_node(AtomicAdjust::compare_and_exchange_ptr((void * TVOLATILE &)_deleted_chain, (void *)next, (void *)obj));
    // Keep trying until no one else got to _deleted_chain first.
  } while (result != next);

#endif  // DELETED_CHAIN_USE_ATOMIC_EXCHANGE
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
  TVOLATILE const ObjectNode *obj = type_to_node((Type *)ptr);
  return AtomicAdjust::get(obj->_flag) == DCF_alive;
#else
  return true;
#endif  // NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function: DeletedChain::node_to_type
//       Access: Private, Static
//  Description: Casts an ObjectNode* to a Type*.
////////////////////////////////////////////////////////////////////
template<class Type>
INLINE Type *DeletedChain<Type>::
node_to_type(TVOLATILE TYPENAME DeletedChain<Type>::ObjectNode *node) {
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

#ifndef DELETED_CHAIN_USE_ATOMIC_EXCHANGE
////////////////////////////////////////////////////////////////////
//     Function: DeletedChain::init_lock
//       Access: Private, Static
//  Description: Ensures the lock pointer has been allocated.
////////////////////////////////////////////////////////////////////
template<class Type>
INLINE void DeletedChain<Type>::
init_lock() {
  if (_lock == (MutexImpl *)NULL) {
    do_init_lock(_lock);
  }
}
#endif  // DELETED_CHAIN_USE_ATOMIC_EXCHANGE

#ifndef DELETED_CHAIN_USE_ATOMIC_EXCHANGE
////////////////////////////////////////////////////////////////////
//     Function: DeletedChain::do_init_lock
//       Access: Private, Static
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
do_init_lock(MutexImpl *lock) {
  MutexImpl *new_lock = new MutexImpl;

  // Even though DELETED_CHAIN_USE_ATOMIC_EXCHANGE is not true, we
  // will take advantage of the atomic exchange operation here, at
  // startup.  We have to rely on something, after all, before we have
  // created the first mutex.
  MutexImpl *result;
  result = (MutexImpl *)AtomicAdjust::compare_and_exchange_ptr((void * TVOLATILE &)lock, (void *)NULL, (void *)new_lock);
  
  if (result != NULL) {
    delete new_lock;
  }
  
  assert(lock != (MutexImpl *)NULL);
}
#endif  // DELETED_CHAIN_USE_ATOMIC_EXCHANGE
