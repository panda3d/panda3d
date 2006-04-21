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
allocate(size_t size) {
  TAU_PROFILE("Type *DeletedChain<Type>::allocate(size_t)", " ", TAU_USER);
  assert(size <= sizeof(Type));

  TVOLATILE ObjectNode *obj;

#ifndef DELETED_CHAIN_USE_ATOMIC_EXCHANGE
  init_lock();
  _lock->lock();
  if (_deleted_chain != (ObjectNode *)NULL) {
    obj = _deleted_chain;
    _deleted_chain = _deleted_chain->_next;
    _lock->release();
#ifndef NDEBUG
    assert(obj->_flag == ((PN_int32)obj ^ deleted_chain_flag_hash));
    obj->_flag = 0;
#endif  // NDEBUG
    return (Type *)obj;
  }
  _lock->release();

#else  // DELETED_CHAIN_USE_ATOMIC_EXCHANGE
  obj = _deleted_chain;
  while (obj != (ObjectNode *)NULL) {
    TVOLATILE ObjectNode *next = obj->_next;
    TVOLATILE ObjectNode *result = (ObjectNode *)AtomicAdjust::compare_and_exchange_ptr((void * TVOLATILE &)_deleted_chain, (void *)obj, (void *)next);
    if (result == obj) {
      // We got it.
#ifndef NDEBUG
      assert(obj->_flag == ((PN_int32)obj ^ deleted_chain_flag_hash));
      obj->_flag = 0;
#endif  // NDEBUG
      return (Type *)obj;
    }
    // Someone else grabbed the top link first.  Try again.
    obj = _deleted_chain;
  }

#endif  // DELETED_CHAIN_USE_ATOMIC_EXCHANGE

  // If we get here, the deleted_chain is empty; we have to allocate a
  // new object from the system pool.

#ifdef DO_MEMORY_USAGE
  obj = (ObjectNode *)(*global_operator_new)(max(sizeof(Type), sizeof(ObjectNode)));
#else
  obj = (ObjectNode *)malloc(max(sizeof(Type), sizeof(ObjectNode)));
#endif
#ifndef NDEBUG
  obj->_flag = 0;
#endif  // NDEBUG

  return (Type *)obj;
}

////////////////////////////////////////////////////////////////////
//     Function: DeletedChain::deallocate
//       Access: Public
//  Description: Frees the memory for an object of Type.
////////////////////////////////////////////////////////////////////
template<class Type>
INLINE void DeletedChain<Type>::
deallocate(Type *ptr) {
  TAU_PROFILE("void DeletedChain<Type>::deallocate(Type *)", " ", TAU_USER);
  TVOLATILE ObjectNode *obj = (ObjectNode *)ptr;

#ifndef NDEBUG
  assert(obj->_flag != ((PN_int32)obj ^ deleted_chain_flag_hash));
  obj->_flag = (PN_int32)obj ^ deleted_chain_flag_hash;
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
    result = (ObjectNode *)AtomicAdjust::compare_and_exchange_ptr((void * TVOLATILE &)_deleted_chain, (void *)next, (void *)obj);
    // Keep trying until no one else got to _deleted_chain first.
  } while (result != next);

#endif  // DELETED_CHAIN_USE_ATOMIC_EXCHANGE
}

#ifndef DELETED_CHAIN_USE_ATOMIC_EXCHANGE
////////////////////////////////////////////////////////////////////
//     Function: DeletedChain::init_lock
//       Access: Private
//  Description: Ensures the lock pointer has been allocated.
////////////////////////////////////////////////////////////////////
template<class Type>
INLINE void DeletedChain<Type>::
init_lock() {
  if (_lock == (MutexImpl *)NULL) {
    _lock = new MutexImpl;
  }
}
#endif  // DELETED_CHAIN_USE_ATOMIC_EXCHANGE
