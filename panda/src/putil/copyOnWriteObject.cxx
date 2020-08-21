/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file copyOnWriteObject.cxx
 * @author drose
 * @date 2007-04-09
 */

#include "copyOnWriteObject.h"
#include "mutexHolder.h"
#include "pnotify.h"

TypeHandle CopyOnWriteObject::_type_handle;

#ifdef COW_THREADED
/**
 * Explicitly decrements the reference count.  See ReferenceCount::unref().
 *
 * In the case of a CopyOnWriteObject, when the reference count decrements
 * down to the cache reference count, the object is implicitly unlocked.
 */
bool CopyOnWriteObject::
unref() const {
  MutexHolder holder(_lock_mutex);
  bool is_zero = CachedTypedWritableReferenceCount::unref();
  if (get_ref_count() == get_cache_ref_count()) {
    ((CopyOnWriteObject *)this)->_lock_status = LS_unlocked;
    ((CopyOnWriteObject *)this)->_locking_thread = nullptr;
    ((CopyOnWriteObject *)this)->_lock_cvar.notify();
  }
  return is_zero;
}

/**
 * Explicitly increments the cache reference count only.  Don't use this.
 *
 * In the case of a CopyOnWriteObject, when the reference count decrements
 * down to the cache reference count, the object is implicitly unlocked.
 */
void CopyOnWriteObject::
cache_ref_only() const {
  MutexHolder holder(_lock_mutex);
  CachedTypedWritableReferenceCount::cache_ref_only();
  if (get_ref_count() == get_cache_ref_count()) {
    ((CopyOnWriteObject *)this)->_lock_status = LS_unlocked;
    ((CopyOnWriteObject *)this)->_locking_thread = nullptr;
    ((CopyOnWriteObject *)this)->_lock_cvar.notify();
  }
}
#endif  // COW_THREADED
