/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file weakReferenceList.h
 * @author drose
 * @date 2004-09-27
 */

#ifndef WEAKREFERENCELIST_H
#define WEAKREFERENCELIST_H

#include "pandabase.h"
#include "pmap.h"
#include "mutexImpl.h"

class WeakPointerCallback;

/**
 * This is an object shared by all the weak pointers that point to the same
 * ReferenceCount object.  It is created whenever a weak reference to an
 * object is created, and can outlive the object until all weak references
 * have disappeared.
 */
class EXPCL_PANDA_EXPRESS WeakReferenceList {
public:
  WeakReferenceList();
  ~WeakReferenceList();

  INLINE void ref() const;
  INLINE bool unref() const;
  INLINE bool was_deleted() const;

  void add_callback(WeakPointerCallback *callback, void *data);
  void remove_callback(WeakPointerCallback *callback);

private:
  void mark_deleted();

public:
  // This lock protects the callbacks below, but it also protects the object
  // from being deleted during a call to WeakPointerTo::lock().
  MutexImpl _lock;

private:
  typedef pmap<WeakPointerCallback *, void *> Callbacks;
  Callbacks _callbacks;

  // This has a very large number added to it if the object is still alive.
  // It could be 1, but having it be a large number makes it easy to check
  // whether the object has been deleted or not.
  static const AtomicAdjust::Integer _alive_offset = (1 << 30);
  mutable AtomicAdjust::Integer _count;

  friend class ReferenceCount;
};

#include "weakReferenceList.I"

#endif
