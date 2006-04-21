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
//     Function: MutexDebug::Copy Constructor
//       Access: Private
//  Description: Do not attempt to copy mutexes.
////////////////////////////////////////////////////////////////////
INLINE MutexDebug::
MutexDebug(const MutexDebug &copy) : _cvar_impl(*get_global_lock()) {
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
  TAU_PROFILE("void MutexDebug::lock()", " ", TAU_USER);
  _global_lock->lock();
  ((MutexDebug *)this)->do_lock();
  _global_lock->release();
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDebug::lock
//       Access: Public
//  Description: This variant on lock() accepts the current thread as
//               a parameter, if it is already known, as an
//               optimization.
////////////////////////////////////////////////////////////////////
INLINE void MutexDebug::
lock(Thread *current_thread) const {
  TAU_PROFILE("void MutexDebug::lock(Thread *)", " ", TAU_USER);
  nassertv(current_thread == Thread::get_current_thread());
  // You may only pass a Thread parameter to a ReMutex--that is, to a
  // mutex whose _allow_recursion flag is true.
  nassertv(_allow_recursion);
  lock();
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDebug::elevate_lock
//       Access: Public
//  Description: This method increments the lock count, assuming the
//               calling thread already holds the lock.  After this
//               call, release() will need to be called one additional
//               time to release the lock.
//
//               This method really performs the same function as
//               lock(), but it offers a potential (slight)
//               performance benefit when the calling thread knows
//               that it already holds the lock.  It is an error to
//               call this when the calling thread does not hold the
//               lock.
////////////////////////////////////////////////////////////////////
INLINE void MutexDebug::
elevate_lock() const {
  TAU_PROFILE("void MutexDebug::elevate_lock()", " ", TAU_USER);
  // You may only pass call elevate_lock() on a ReMutex--that is, to a
  // mutex whose _allow_recursion flag is true.
  nassertv(_allow_recursion);

  // Also, it's an error to call this if the lock is not already held.
  nassertv(debug_is_locked());

  lock();
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
  TAU_PROFILE("void MutexDebug::release()", " ", TAU_USER);
  _global_lock->lock();
  ((MutexDebug *)this)->do_release();
  _global_lock->release();
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
  TAU_PROFILE("bool MutexDebug::debug_is_locked()", " ", TAU_USER);
  _global_lock->lock();
  bool is_locked = do_debug_is_locked();
  _global_lock->release();
  return is_locked;
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDebug::get_global_lock
//       Access: Private, Static
//  Description: Ensures the global MutexImpl pointer has been
//               created, and returns its pointer.  Since this method
//               is called by the MutexDebug constructor, any other
//               (non-static) methods of MutexDebug may simply assume
//               that the pointer has already been created.
////////////////////////////////////////////////////////////////////
INLINE MutexImpl *MutexDebug::
get_global_lock() {
  if (_global_lock == (MutexImpl *)NULL) {
    _global_lock = new MutexImpl;
  }
  return _global_lock;
}
