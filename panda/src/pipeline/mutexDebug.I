// Filename: mutexDebug.I
// Created by:  drose (13Feb06)
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
//     Function: MutexDebug::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MutexDebug::
MutexDebug(bool allow_recursion) :
  _allow_recursion(allow_recursion),
  _locking_thread(NULL),
  _lock_count(0),
  _cvar(_global_mutex)
{
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDebug::Copy Constructor
//       Access: Private
//  Description: Do not attempt to copy mutexes.
////////////////////////////////////////////////////////////////////
INLINE MutexDebug::
MutexDebug(const MutexDebug &copy) : _cvar(_global_mutex) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDebug::Copy Assignment Operator
//       Access: Private
//  Description: Do not attempt to copy mutexes.
////////////////////////////////////////////////////////////////////
INLINE void MutexDebug::
operator = (const MutexDebug &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDebug::lock
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
INLINE void MutexDebug::
lock() const {
  _global_mutex.lock();
  ((MutexDebug *)this)->do_lock();
  _global_mutex.release();
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDebug::release
//       Access: Public
//  Description: Releases the mutex.  It is an error to call this if
//               the mutex was not already locked.
//
//               This method is considered const so that you can lock
//               and unlock const mutexes, mainly to allow thread-safe
//               access to otherwise const data.
////////////////////////////////////////////////////////////////////
INLINE void MutexDebug::
release() const {
  _global_mutex.lock();
  ((MutexDebug *)this)->do_release();
  _global_mutex.release();
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDebug::debug_is_locked
//       Access: Public
//  Description: Returns true if the current thread has locked the
//               Mutex, false otherwise.  This method is only intended
//               for use in debugging, hence the method name; in the
//               MutexDebug case, it always returns true, since
//               there's not a reliable way to determine this
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool MutexDebug::
debug_is_locked() const {
  _global_mutex.lock();
  bool is_locked = do_debug_is_locked();
  _global_mutex.release();
  return is_locked;
}
