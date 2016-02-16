/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file weakPointerToVoid.I
 * @author drose
 * @date 2004-09-27
 */

/**

 */
INLINE WeakPointerToVoid::
WeakPointerToVoid() {
  _ptr_was_deleted = false;
  _callback = NULL;
}

/**

 */
INLINE WeakPointerToVoid::
~WeakPointerToVoid() {
}

/**
 * This is intended only to be called by the WeakPointerList destructor.  It
 * indicates that the object that we were pointing to has just been deleted.
 */
INLINE void WeakPointerToVoid::
mark_deleted() {
  nassertv(!_ptr_was_deleted);
  _ptr_was_deleted = true;
  if (_callback != (WeakPointerCallback *)NULL) {
    _callback->wp_callback(_void_ptr);
  }
}

/**
 * Sets a callback that will be made when the pointer is deleted.  If a previous
 * callback has already been set, it will be replaced.  If the pointer has
 * already been deleted, the callback will be made immediately.
 */
INLINE void WeakPointerToVoid::
set_callback(WeakPointerCallback *callback) {
  _callback = callback;
  if (_ptr_was_deleted && _callback != (WeakPointerCallback *)NULL) {
    _callback->wp_callback(_void_ptr);
  }
}

/**
 * Returns the callback that will be made when the pointer is deleted, or NULL
 * if no callback has been set.
 */
INLINE WeakPointerCallback *WeakPointerToVoid::
get_callback() const {
  return _callback;
}

/**
 * Returns true if the object we are pointing to has been deleted, false
 * otherwise.
 */
INLINE bool WeakPointerToVoid::
was_deleted() const {
  return _ptr_was_deleted;
}

/**
 * Returns true if the pointer is not null and the object has not been deleted.
 */
INLINE bool WeakPointerToVoid::
is_valid_pointer() const {
  return (_void_ptr != (void *)NULL) && !_ptr_was_deleted;
}
