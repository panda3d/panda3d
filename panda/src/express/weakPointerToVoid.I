// Filename: weakPointerToVoid.I
// Created by:  drose (27Sep04)
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


////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToVoid::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE WeakPointerToVoid::
WeakPointerToVoid() {
  _ptr_was_deleted = false;
  _callback = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToVoid::Destructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE WeakPointerToVoid::
~WeakPointerToVoid() {
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToVoid::mark_deleted
//       Access: Public
//  Description: This is intended only to be called by the
//               WeakPointerList destructor.  It indicates that the
//               object that we were pointing to has just been
//               deleted.
////////////////////////////////////////////////////////////////////
INLINE void WeakPointerToVoid::
mark_deleted() {
  nassertv(!_ptr_was_deleted);
  _ptr_was_deleted = true;
  if (_callback != (WeakPointerCallback *)NULL) {
    _callback->wp_callback(_void_ptr);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToVoid::set_callback
//       Access: Public
//  Description: Sets a callback that will be made when the pointer is
//               deleted.  If a previous callback has already been
//               set, it will be replaced.
//
//               If the pointer has already been deleted, the callback
//               will be made immediately.
////////////////////////////////////////////////////////////////////
INLINE void WeakPointerToVoid::
set_callback(WeakPointerCallback *callback) {
  _callback = callback;
  if (_ptr_was_deleted && _callback != (WeakPointerCallback *)NULL) {
    _callback->wp_callback(_void_ptr);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToVoid::get_callback
//       Access: Public
//  Description: Returns the callback that will be made when the
//               pointer is deleted, or NULL if no callback has been
//               set.
////////////////////////////////////////////////////////////////////
INLINE WeakPointerCallback *WeakPointerToVoid::
get_callback() const {
  return _callback;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToVoid::was_deleted
//       Access: Published
//  Description: Returns true if the object we are pointing to has
//               been deleted, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool WeakPointerToVoid::
was_deleted() const {
  return _ptr_was_deleted;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToVoid::is_valid_pointer
//       Access: Published
//  Description: Returns true if the pointer is not null and the
//               object has not been deleted.
////////////////////////////////////////////////////////////////////
INLINE bool WeakPointerToVoid::
is_valid_pointer() const {
  return (_void_ptr != (void *)NULL) && !_ptr_was_deleted;
}
