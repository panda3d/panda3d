/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file copyOnWritePointer.cxx
 * @author drose
 * @date 2007-04-09
 */

#include "copyOnWritePointer.h"
#include "config_putil.h"
#include "config_pipeline.h"

#ifdef COW_THREADED
/**
 * Returns a pointer locked for read.  Until this pointer dereferences, calls
 * to get_write_pointer() will force a copy.
 *
 * This flavor of the method is written for the threaded case.
 */
CPT(CopyOnWriteObject) CopyOnWritePointer::
get_read_pointer(Thread *current_thread) const {
  if (_cow_object == nullptr) {
    return nullptr;
  }

  MutexHolder holder(_cow_object->_lock_mutex);
  while (_cow_object->_lock_status == CopyOnWriteObject::LS_locked_write) {
    if (_cow_object->_locking_thread == current_thread) {
      return _cow_object;
    }
    if (util_cat.is_debug()) {
      util_cat.debug()
        << *current_thread << " waiting on " << _cow_object->get_type()
        << " " << _cow_object << ", held by " << *_cow_object->_locking_thread
        << "\n";
    }
    _cow_object->_lock_cvar.wait();
  }

  _cow_object->_lock_status = CopyOnWriteObject::LS_locked_read;
  _cow_object->_locking_thread = current_thread;
  return _cow_object;
}
#endif  // COW_THREADED

#ifdef COW_THREADED
/**
 * Returns a pointer locked for write.  If another thread or threads already
 * hold the pointer locked for read, then this will force a copy.
 *
 * Until this pointer dereferences, calls to get_read_pointer() or
 * get_write_pointer() will block.
 *
 * This flavor of the method is written for the threaded case.
 */
PT(CopyOnWriteObject) CopyOnWritePointer::
get_write_pointer() {
  if (_cow_object == nullptr) {
    return nullptr;
  }

  Thread *current_thread = Thread::get_current_thread();

  _cow_object->_lock_mutex.lock();
  while (_cow_object->_lock_status == CopyOnWriteObject::LS_locked_write &&
         _cow_object->_locking_thread != current_thread) {
    if (util_cat.is_debug()) {
      util_cat.debug()
        << *current_thread << " waiting on " << _cow_object->get_type()
        << " " << _cow_object << ", held by " << *_cow_object->_locking_thread
        << "\n";
    }
    _cow_object->_lock_cvar.wait();
  }

  if (_cow_object->_lock_status == CopyOnWriteObject::LS_locked_read) {
    nassertr(_cow_object->get_ref_count() > _cow_object->get_cache_ref_count(), nullptr);

    if (util_cat.is_debug()) {
      util_cat.debug()
        << "Making copy of " << _cow_object->get_type()
        << " because it is locked in read mode.\n";
    }

    PT(CopyOnWriteObject) new_object = _cow_object->make_cow_copy();
    _cow_object->CachedTypedWritableReferenceCount::cache_unref();
    _cow_object->_lock_mutex.unlock();

    MutexHolder holder(new_object->_lock_mutex);
    _cow_object = new_object;
    _cow_object->CachedTypedWritableReferenceCount::cache_ref();
    _cow_object->_lock_status = CopyOnWriteObject::LS_locked_write;
    _cow_object->_locking_thread = current_thread;

    return new_object;

  } else if (_cow_object->get_cache_ref_count() > 1) {
    // No one else has it specifically read-locked, but there are other
    // CopyOnWritePointers holding the same object, so we should make our own
    // writable copy anyway.
    if (util_cat.is_debug()) {
      util_cat.debug()
        << "Making copy of " << _cow_object->get_type()
        << " because it is shared by " << _cow_object->get_ref_count()
        << " pointers.\n";
    }

    PT(CopyOnWriteObject) new_object = _cow_object->make_cow_copy();
    _cow_object->CachedTypedWritableReferenceCount::cache_unref();
    _cow_object->_lock_mutex.unlock();

    MutexHolder holder(new_object->_lock_mutex);
    _cow_object = new_object;
    _cow_object->CachedTypedWritableReferenceCount::cache_ref();
    _cow_object->_lock_status = CopyOnWriteObject::LS_locked_write;
    _cow_object->_locking_thread = current_thread;

    return new_object;

  } else {
    // No other thread has the pointer locked, and we're the only
    // CopyOnWritePointer with this object.  We can safely write to it without
    // making a copy.

    // We can't assert that there are no outstanding ordinary references to
    // it, though, since the creator of the object might have saved himself a
    // reference.
    _cow_object->_lock_status = CopyOnWriteObject::LS_locked_write;
    _cow_object->_locking_thread = current_thread;
    _cow_object->_lock_mutex.unlock();
  }

  return _cow_object;
}
#endif  // COW_THREADED
