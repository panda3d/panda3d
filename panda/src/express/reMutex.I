// Filename: reMutex.I
// Created by:  drose (15Jan06)
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
//     Function: ReMutex::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ReMutex::
ReMutex() : _cvar(_mutex) {
  _locking_thread = NULL;
  _lock_count = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutex::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ReMutex::
~ReMutex() {
  nassertv(_locking_thread == NULL && _lock_count == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutex::Copy Constructor
//       Access: Private
//  Description: Do not attempt to copy mutexes.
////////////////////////////////////////////////////////////////////
INLINE ReMutex::
ReMutex(const ReMutex &copy) : _cvar(_mutex) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutex::Copy Assignment Operator
//       Access: Private
//  Description: Do not attempt to copy mutexes.
////////////////////////////////////////////////////////////////////
INLINE void ReMutex::
operator = (const ReMutex &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutex::lock
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
//               Also see ReMutexHolder.
////////////////////////////////////////////////////////////////////
INLINE void ReMutex::
lock() const {
  ((ReMutex *)this)->do_lock();
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutex::release
//       Access: Public
//  Description: Releases the mutex.  It is an error to call this if
//               the mutex was not already locked.
//
//               This method is considered const so that you can lock
//               and unlock const mutexes, mainly to allow thread-safe
//               access to otherwise const data.
////////////////////////////////////////////////////////////////////
INLINE void ReMutex::
release() const {
  ((ReMutex *)this)->do_release();
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutex::is_locked
//       Access: Public
//  Description: Returns true if *this thread* already holds the lock,
//               false if no one holds the lock or the lock is held by
//               some other thread.
////////////////////////////////////////////////////////////////////
INLINE bool ReMutex::
is_locked() const {
  MutexHolder holder(_mutex);
  return (_locking_thread == Thread::get_current_thread());
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutex::get_lock_count
//       Access: Public
//  Description: Returns the number of times that *this thread* has
//               grabbed the lock.  Returns 0 if some other thread is
//               currently holding the lock.
////////////////////////////////////////////////////////////////////
INLINE int ReMutex::
get_lock_count() const {
  MutexHolder holder(_mutex);
  if (_locking_thread == Thread::get_current_thread()) {
    return _lock_count;
  }
  return 0;
}

#ifndef NDEBUG
////////////////////////////////////////////////////////////////////
//     Function: ReMutex::debug_is_locked
//       Access: Public
//  Description: The same as is_locked().  This method name is
//               provided just to maintain symmetry with Mutex (which
//               doesn't provide an is_locked() method).
////////////////////////////////////////////////////////////////////
INLINE bool ReMutex::
debug_is_locked() const {
  return is_locked();
}
#endif  // NDEBUG
