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
#include "thread.h"
#include "epochHolder.h"

#ifdef COW_THREADED
/**
 * Returns a pointer locked for read.  Until this pointer dereferences, calls
 * to get_write_pointer() will force a copy.
 *
 * This flavor of the method is written for the threaded case.
 */
CPT(CopyOnWriteObject) CopyOnWritePointer::
get_read_pointer(Thread *current_thread) const {
  // Pin _cow_object in a strong ref before locking its mutex: another thread's
  // get_write_pointer() can swap _cow_object out and free the old object,
  // taking _lock_mutex with it.  Declared before `holder` so it outlives the
  // lock (reverse destruction order).
  PT(CopyOnWriteObject) result = _cow_object;
  if (result == nullptr) {
    return nullptr;
  }

  MutexHolder holder(result->_lock_mutex);
  while (result->_lock_status == CopyOnWriteObject::LS_locked_write) {
    if (result->_locking_thread == current_thread) {
      return result;
    }
    if (util_cat.is_debug()) {
      util_cat.debug()
        << *current_thread << " waiting on " << result->get_type()
        << " " << result.p() << ", held by " << *result->_locking_thread
        << "\n";
    }
    result->_lock_cvar.wait();
  }

  result->_lock_status = CopyOnWriteObject::LS_locked_read;
  result->_locking_thread = current_thread;
  return result;
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
  // Pin _cow_object before locking — see get_read_pointer().  Without the pin,
  // another thread can swap and free the old object before our explicit
  // unlock, leaving us unlocking freed memory.
  PT(CopyOnWriteObject) old_object = _cow_object;
  if (old_object == nullptr) {
    return nullptr;
  }

  Thread *current_thread = Thread::get_current_thread();

  // Defer any EBR reclamation triggered inside make_cow_copy() until after we
  // release _lock_mutex below -- otherwise reclamation can re-enter this same
  // lock and self-deadlock.  Declared before the lock so it outlives every
  // MutexHolder in the branches below (reverse destruction order).
  EpochHolder reclaim_deferral(current_thread);

  old_object->_lock_mutex.lock();
  while (old_object->_lock_status == CopyOnWriteObject::LS_locked_write &&
         old_object->_locking_thread != current_thread) {
    if (util_cat.is_debug()) {
      util_cat.debug()
        << *current_thread << " waiting on " << old_object->get_type()
        << " " << old_object.p() << ", held by " << *old_object->_locking_thread
        << "\n";
    }
    old_object->_lock_cvar.wait();
  }

  if (old_object->_lock_status == CopyOnWriteObject::LS_locked_read) {
    nassertr(old_object->get_ref_count() > old_object->get_cache_ref_count(), nullptr);

    if (util_cat.is_debug()) {
      util_cat.debug()
        << "Making copy of " << old_object->get_type()
        << " because it is locked in read mode.\n";
    }

    PT(CopyOnWriteObject) new_object = old_object->make_cow_copy();
    old_object->CachedTypedWritableReferenceCount::cache_unref();
    old_object->_lock_mutex.unlock();

    MutexHolder holder(new_object->_lock_mutex);
    _cow_object = new_object;
    _cow_object->CachedTypedWritableReferenceCount::cache_ref();
    _cow_object->_lock_status = CopyOnWriteObject::LS_locked_write;
    _cow_object->_locking_thread = current_thread;

    return new_object;

  } else if (old_object->get_cache_ref_count() > 1) {
    // No one else has it specifically read-locked, but there are other
    // CopyOnWritePointers holding the same object, so we should make our own
    // writable copy anyway.
    if (util_cat.is_debug()) {
      util_cat.debug()
        << "Making copy of " << old_object->get_type()
        << " because it is shared by " << old_object->get_ref_count()
        << " pointers.\n";
    }

    PT(CopyOnWriteObject) new_object = old_object->make_cow_copy();
    old_object->CachedTypedWritableReferenceCount::cache_unref();
    old_object->_lock_mutex.unlock();

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
    old_object->_lock_status = CopyOnWriteObject::LS_locked_write;
    old_object->_locking_thread = current_thread;
    old_object->_lock_mutex.unlock();
  }

  return old_object;
}
#endif  // COW_THREADED
