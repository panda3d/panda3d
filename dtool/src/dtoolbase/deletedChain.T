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
TYPENAME DeletedChain<Type>::ObjectNode *DeletedChain<Type>::_deleted_chain;

#ifndef HAVE_ATOMIC_COMPARE_AND_EXCHANGE_PTR
template<class Type>
MutexImpl DeletedChain<Type>::_lock;
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

#ifndef HAVE_ATOMIC_COMPARE_AND_EXCHANGE_PTR
  _lock.lock();
  if (_deleted_chain != (ObjectNode *)NULL) {
    ObjectNode *obj = _deleted_chain;
    _deleted_chain = _deleted_chain->_next;
    _lock.release();
    return (Type *)obj;
  }
  _lock.release();

#else  // HAVE_ATOMIC_COMPARE_AND_EXCHANGE_PTR
  ObjectNode *obj = _deleted_chain;
  while (obj != (ObjectNode *)NULL) {
    ObjectNode *result = (ObjectNode *)AtomicAdjust::compare_and_exchange_ptr((void *&)_deleted_chain, (void *)obj, (void *)obj->_next);
    if (result == obj) {
      // We got it.
      return (Type *)obj;
    }
    // Someone else grabbed the top link first.  Try again.
    obj = _deleted_chain;
  }

#endif  // HAVE_ATOMIC_COMPARE_AND_EXCHANGE_PTR

  // If we get here, the deleted_chain is empty; we have to allocate a
  // new object from the system pool.

#ifdef DO_MEMORY_USAGE
  return (Type *)(*global_operator_new)(sizeof(Type));
#else
  return (Type *)malloc(sizeof(Type));
#endif
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
#ifndef HAVE_ATOMIC_COMPARE_AND_EXCHANGE_PTR
  _lock.lock();

  ObjectNode *obj = (ObjectNode *)ptr;
  obj->_next = _deleted_chain;
  _deleted_chain = obj;

  _lock.release();

#else  // HAVE_ATOMIC_COMPARE_AND_EXCHANGE_PTR

  ObjectNode *obj = (ObjectNode *)ptr;
  ObjectNode *result;

  do {
    obj->_next = _deleted_chain;
    result = (ObjectNode *)AtomicAdjust::compare_and_exchange_ptr((void *&)_deleted_chain, (void *)obj->_next, (void *)obj);
    // Keep trying until no one else got to _deleted_chain first.
  } while (result != obj->_next);

#endif  // HAVE_ATOMIC_COMPARE_AND_EXCHANGE_PTR
}
