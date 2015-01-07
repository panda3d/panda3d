// Filename: typeRegistry.I
// Created by:  drose (06Aug01)
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


////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::freshen_derivations
//       Access: Private
//  Description: Rebuilds the derivation data structures after some
//               derivation relationship has been modified, so that
//               class relationships can quickly be determined.
////////////////////////////////////////////////////////////////////
INLINE void TypeRegistry::
freshen_derivations() {
  if (!_derivations_fresh) {
    rebuild_derivations();
    _derivations_fresh = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::init_lock
//       Access: Private, Static
//  Description: Ensures the lock pointer has been allocated.
////////////////////////////////////////////////////////////////////
INLINE void TypeRegistry::
init_lock() {
  if (_lock == (MutexImpl *)NULL) {
    _lock = new MutexImpl;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::look_up
//       Access: Private
//  Description: Returns the TypeRegistryNode associated with the
//               indicated TypeHandle.  If there is no associated
//               TypeRegistryNode, reports an error condition and
//               returns NULL.
//
//               The associated TypedObject pointer is the pointer to
//               the object that owns the handle, if available.  It is
//               only used in an error condition, if for some reason
//               the handle was uninitialized.
//
//               Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
INLINE TypeRegistryNode *TypeRegistry::
look_up(TypeHandle handle, TypedObject *object) const {
#ifndef NDEBUG
  if (handle._index >= (int)_handle_registry.size() ||
      handle._index <= 0) {
    // Invalid or uninitialized type handle.
    return look_up_invalid(handle, object);
  }
#endif
  return _handle_registry[handle._index];
}
