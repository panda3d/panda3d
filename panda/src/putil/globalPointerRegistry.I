/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file globalPointerRegistry.I
 * @author drose
 * @date 2000-02-03
 */

/**
 * Returns the pointer associated with the indicated TypeHandle, if any.  If
 * no pointer has yet been associated, returns NULL.
 */
INLINE void *GlobalPointerRegistry::
get_pointer(TypeHandle type) {
  return get_global_ptr()->ns_get_pointer(type);
}

/**
 * Associates the given pointer with the indicated TypeHandle.  It is an error
 * to call this with a NULL pointer, or to call this function more than once
 * with a given TypeHandle (without first calling clear_pointer).
 */
INLINE void GlobalPointerRegistry::
store_pointer(TypeHandle type, void *ptr) {
  get_global_ptr()->ns_store_pointer(type, ptr);
}

/**
 * Removes the association of the given pointer with the indicated TypeHandle.
 * Subsequent calls to get_pointer() with this TypeHandle will return NULL,
 * until another call to store_pointer() is made.
 */
INLINE void GlobalPointerRegistry::
clear_pointer(TypeHandle type) {
  get_global_ptr()->ns_clear_pointer(type);
}


/**
 * Returns a pointer to the single GlobalPointerRegistry object.  If the
 * object does not yet exist, creates it.  This indirection is used instead of
 * making all the data members of GlobalPointerRegistry static, so that we
 * don't have to worry about order dependency during static init time.
 */
INLINE GlobalPointerRegistry *GlobalPointerRegistry::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    _global_ptr = new GlobalPointerRegistry;
  }
  return _global_ptr;
}
