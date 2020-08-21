/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file weakReferenceList.cxx
 * @author drose
 * @date 2004-09-27
 */

#include "weakReferenceList.h"
#include "weakPointerToVoid.h"
#include "pnotify.h"

/**
 *
 */
WeakReferenceList::
WeakReferenceList() : _count(_alive_offset) {
}

/**
 * The destructor tells all of the owned references that we're gone.
 */
WeakReferenceList::
~WeakReferenceList() {
  nassertv(_count == 0);
}

/**
 * Adds the callback to the list of callbacks that will be called when the
 * underlying pointer is deleted.  If it has already been deleted, it will
 * be called immediately.
 *
 * The data pointer can be an arbitrary pointer and is passed as only argument
 * to the callback.
 *
 * @since 1.10.0
 */
void WeakReferenceList::
add_callback(WeakPointerCallback *callback, void *data) {
  nassertv(callback != nullptr);
  _lock.lock();
  // We need to check again whether the object is deleted after grabbing the
  // lock, despite having already done this in weakPointerTo.I, since it may
  // have been deleted in the meantime.
  bool deleted = was_deleted();
  if (!deleted) {
    _callbacks.insert(std::make_pair(callback, data));
  }
  _lock.unlock();

  if (deleted) {
    callback->wp_callback(data);
  }
}

/**
 * Intended to be called only by WeakPointerTo (or by any class implementing a
 * weak reference-counting pointer), this removes the indicated PointerToVoid
 * structure from the list of such structures that are maintaining a weak
 * pointer to this object.
 *
 * @since 1.10.0
 */
void WeakReferenceList::
remove_callback(WeakPointerCallback *callback) {
  nassertv(callback != nullptr);
  _lock.lock();
  _callbacks.erase(callback);
  _lock.unlock();
}

/**
 * Called only by the ReferenceCount pointer to indicate that it has been
 * deleted.
 *
 * @since 1.10.0
 */
void WeakReferenceList::
mark_deleted() {
  _lock.lock();
  Callbacks::iterator ci;
  for (ci = _callbacks.begin(); ci != _callbacks.end(); ++ci) {
    (*ci).first->wp_callback((*ci).second);
  }
  _callbacks.clear();

  // Decrement the special offset added to the weak pointer count to indicate
  // that it can be deleted when all the weak references have gone.
  AtomicAdjust::Integer result = AtomicAdjust::add(_count, -_alive_offset);
  _lock.unlock();
  if (result == 0) {
    // There are no weak references remaining either, so delete this.
    delete this;
  }
  nassertv(result >= 0);
}
