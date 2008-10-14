// Filename: mutexDebug.I
// Created by:  drose (13Feb06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
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
//     Function: MutexDebug::acquire
//       Access: Published
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
acquire(Thread *current_thread) const {
  TAU_PROFILE("void MutexDebug::acquire(Thread *)", " ", TAU_USER);
  nassertv(current_thread == Thread::get_current_thread());
  _global_lock->acquire();
  ((MutexDebug *)this)->do_acquire(current_thread);
  _global_lock->release();
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDebug::try_acquire
//       Access: Published
//  Description: Returns immediately, with a true value indicating the
//               mutex has been acquired, and false indicating it has
//               not.
////////////////////////////////////////////////////////////////////
INLINE bool MutexDebug::
try_acquire(Thread *current_thread) const {
  TAU_PROFILE("void MutexDebug::acquire(Thread *)", " ", TAU_USER);
  nassertr(current_thread == Thread::get_current_thread(), false);
  _global_lock->acquire();
  bool acquired = ((MutexDebug *)this)->do_try_acquire(current_thread);
  _global_lock->release();
  return acquired;
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDebug::elevate_lock
//       Access: Published
//  Description: This method increments the lock count, assuming the
//               calling thread already holds the lock.  After this
//               call, release() will need to be called one additional
//               time to release the lock.
//
//               This method really performs the same function as
//               acquire(), but it offers a potential (slight)
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

  acquire();
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDebug::release
//       Access: Published
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
  _global_lock->acquire();
  ((MutexDebug *)this)->do_release();
  _global_lock->release();
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDebug::debug_is_locked
//       Access: Published
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
  _global_lock->acquire();
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
INLINE MutexTrueImpl *MutexDebug::
get_global_lock() {
  if (_global_lock == (MutexTrueImpl *)NULL) {
    _global_lock = new MutexTrueImpl;
  }
  return _global_lock;
}
