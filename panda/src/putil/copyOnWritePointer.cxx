// Filename: copyOnWritePointer.cxx
// Created by:  drose (09Apr07)
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

#include "copyOnWritePointer.h"
#include "mutexHolder.h"
#include "config_util.h"
#include "config_pipeline.h"

#ifdef COW_THREADED
////////////////////////////////////////////////////////////////////
//     Function: CopyOnWritePointer::get_read_pointer
//       Access: Public
//  Description: Returns a pointer locked for read.  Until this
//               pointer dereferences, calls to get_write_pointer()
//               will force a copy.
//
//               This flavor of the method is written for the threaded
//               case.
////////////////////////////////////////////////////////////////////
CPT(CopyOnWriteObject) CopyOnWritePointer::
get_read_pointer() const {
  if (_object == (CopyOnWriteObject *)NULL) {
    return NULL;
  }

  Thread *current_thread = Thread::get_current_thread();

  MutexHolder holder(_object->_lock_mutex);
  while (_object->_lock_status == CopyOnWriteObject::LS_locked_write) {
    if (_object->_locking_thread == current_thread) {
      return _object;
    }
    if (util_cat.is_debug()) {
      util_cat.debug() 
        << *current_thread << " waiting on " << _object->get_type()
        << " " << _object << ", held by " << *_object->_locking_thread
        << "\n";
    }
    _object->_lock_cvar.wait();
  }

  _object->_lock_status = CopyOnWriteObject::LS_locked_read;
  _object->_locking_thread = current_thread;
  return _object;
}
#endif  // COW_THREADED

#ifdef COW_THREADED
////////////////////////////////////////////////////////////////////
//     Function: CopyOnWritePointer::get_write_pointer
//       Access: Public
//  Description: Returns a pointer locked for write.  If another
//               thread or threads already hold the pointer locked for
//               read, then this will force a copy.
//
//               Until this pointer dereferences, calls to
//               get_read_pointer() or get_write_pointer() will block.
//
//               This flavor of the method is written for the threaded
//               case.
////////////////////////////////////////////////////////////////////
PT(CopyOnWriteObject) CopyOnWritePointer::
get_write_pointer() {
  if (_object == (CopyOnWriteObject *)NULL) {
    return NULL;
  }

  Thread *current_thread = Thread::get_current_thread();

  MutexHolder holder(_object->_lock_mutex);
  while (_object->_lock_status == CopyOnWriteObject::LS_locked_write &&
         _object->_locking_thread != current_thread) {
    if (util_cat.is_debug()) {
      util_cat.debug() 
        << *current_thread << " waiting on " << _object->get_type()
        << " " << _object << ", held by " << *_object->_locking_thread
        << "\n";
    }
    _object->_lock_cvar.wait();
  }

  if (_object->_lock_status == CopyOnWriteObject::LS_locked_read) {
    nassertr(_object->get_ref_count() > _object->get_cache_ref_count(), NULL);

    if (util_cat.is_debug()) {
      util_cat.debug()
        << "Making copy of " << _object->get_type()
        << " because it is locked in read mode.\n";
    }
    PT(CopyOnWriteObject) new_object = _object->make_cow_copy();
    cache_unref_delete(_object);
    _object = new_object;
    _object->cache_ref();

  } else if (_object->get_cache_ref_count() > 1) {
    // No one else has it specifically read-locked, but there are
    // other CopyOnWritePointers holding the same object, so we should
    // make our own writable copy anyway.
    if (util_cat.is_debug()) {
      util_cat.debug()
        << "Making copy of " << _object->get_type()
        << " because it is shared by " << _object->get_ref_count()
        << " pointers.\n";
    }

    PT(CopyOnWriteObject) new_object = _object->make_cow_copy();
    cache_unref_delete(_object);
    _object = new_object;
    _object->cache_ref();

  } else {
    // No other thread has the pointer locked, and we're the only
    // CopyOnWritePointer with this object.  We can safely write to it
    // without making a copy.

    // We can't assert that there are no outstanding ordinary
    // references to it, though, since the creator of the object might
    // have saved himself a reference.
  }
  _object->_lock_status = CopyOnWriteObject::LS_locked_write;
  _object->_locking_thread = current_thread;

  return _object;
}
#endif  // COW_THREADED
