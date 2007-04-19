// Filename: copyOnWriteObject.cxx
// Created by:  drose (09Apr07)
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

#include "copyOnWriteObject.h"
#include "mutexHolder.h"
#include "pnotify.h"

TypeHandle CopyOnWriteObject::_type_handle;

#ifdef HAVE_THREADS
////////////////////////////////////////////////////////////////////
//     Function: CopyOnWriteObject::unref
//       Access: Public
//  Description: Explicitly decrements the reference count.  See
//               ReferenceCount::unref().
//
//               In the case of a CopyOnWriteObject, when the
//               reference count decrements down to the cache
//               reference count, the object is implicitly unlocked.
////////////////////////////////////////////////////////////////////
bool CopyOnWriteObject::
unref() const {
  MutexHolder holder(_lock_mutex);
  bool is_zero = CachedTypedWritableReferenceCount::unref();
  if (get_ref_count() == get_cache_ref_count()) {
    ((CopyOnWriteObject *)this)->_lock_status = LS_unlocked;
    ((CopyOnWriteObject *)this)->_lock_cvar.signal();
  }
  return is_zero;
}
#endif  // HAVE_THREADS
