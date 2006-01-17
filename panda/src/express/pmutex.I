// Filename: pmutex.I
// Created by:  drose (08Aug02)
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
//     Function: Mutex::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Mutex::
Mutex() {
}

////////////////////////////////////////////////////////////////////
//     Function: Mutex::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Mutex::
~Mutex() {
}

////////////////////////////////////////////////////////////////////
//     Function: Mutex::Copy Constructor
//       Access: Private
//  Description: Do not attempt to copy mutexes.
////////////////////////////////////////////////////////////////////
INLINE Mutex::
Mutex(const Mutex &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: Mutex::Copy Assignment Operator
//       Access: Private
//  Description: Do not attempt to copy mutexes.
////////////////////////////////////////////////////////////////////
INLINE void Mutex::
operator = (const Mutex &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: Mutex::lock
//       Access: Public
//  Description: Grabs the mutex if it is available.  If it is not
//               available, blocks until it becomes available, then
//               grabs it.  In either case, the function does not
//               return until the mutex is held; you should then call
//               unlock().
//
//               This method is considered const so that you can lock
//               and unlock const mutexes, mainly to allow thread-safe
//               access to otherwise const data.
//
//               Also see MutexHolder.
////////////////////////////////////////////////////////////////////
INLINE void Mutex::
lock() const {
#ifndef CHECK_REENTRANT_MUTEX
  // In the production case, just lock the thing immediately.  Don't
  // bother with the out-of-line do_lock() method, since we won't be
  // performing any checks anyway.
  ((MutexImpl &)_impl).lock();

#else 
  // In development mode, call the out-of-line do_lock() method, which
  // might perform an additional check or two.
  ((Mutex *)this)->do_lock();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: Mutex::release
//       Access: Public
//  Description: Releases the mutex.  It is an error to call this if
//               the mutex was not already locked.
//
//               This method is considered const so that you can lock
//               and unlock const mutexes, mainly to allow thread-safe
//               access to otherwise const data.
////////////////////////////////////////////////////////////////////
INLINE void Mutex::
release() const {
#ifndef CHECK_REENTRANT_MUTEX
  // In the production case, just release the thing immediately.
  // Don't bother with the out-of-line do_release() method, since we
  // won't be performing any checks anyway.
  ((MutexImpl &)_impl).release();

#else 
  // In development mode, call the out-of-line do_release() method,
  // which might perform an additional check or two.
  ((Mutex *)this)->do_release();
#endif
}

#ifndef CHECK_REENTRANT_MUTEX
////////////////////////////////////////////////////////////////////
//     Function: Mutex::debug_is_locked
//       Access: Public
//  Description: Returns true if the current thread has locked the
//               Mutex, false otherwise.  This method is only
//               meaningful if CHECK_REENTRANT_MUTEX is defined;
//               otherwise, it always returns true.
////////////////////////////////////////////////////////////////////
INLINE bool Mutex::
debug_is_locked() const {
  return true;
}
#endif  // CHECK_REENTRANT_MUTEX
