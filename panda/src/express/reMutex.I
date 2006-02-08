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


#ifdef MUTEX_REENTRANT
// In this branch, the ReMutex class is implemented as a thin wrapper
// around the native MutexImpl class (which is already reentrant
// anyway).

////////////////////////////////////////////////////////////////////
//     Function: ReMutex::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ReMutex::
ReMutex() {
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutex::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ReMutex::
~ReMutex() {
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutex::Copy Constructor
//       Access: Private
//  Description: Do not attempt to copy mutexes.
////////////////////////////////////////////////////////////////////
INLINE ReMutex::
ReMutex(const ReMutex &copy) {
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
//               Also see MutexHolder.
////////////////////////////////////////////////////////////////////
INLINE void ReMutex::
lock() const {
  ((MutexImpl &)_impl).lock();
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
  ((MutexImpl &)_impl).release();
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutex::debug_is_locked
//       Access: Public
//  Description: Returns true if the current thread has locked the
//               Mutex, false otherwise.  This method is only
//               meaningful if CHECK_REENTRANT_MUTEX is defined;
//               otherwise, it always returns true.
////////////////////////////////////////////////////////////////////
INLINE bool ReMutex::
debug_is_locked() const {
  return true;
}


#elif !defined(THREAD_DUMMY_IMPL) || defined(CHECK_REENTRANT_MUTEX)

// In this branch, the ReMutex object is implemented as a heavy
// wrapper around the Mutex and ConditionVar classes, to impose
// reentrant semantics where the underlying MutexImpl doesn't provide
// them.

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
//     Function: ReMutex::debug_is_locked
//       Access: Public
//  Description: Returns true if *this thread* already holds the lock,
//               false if no one holds the lock or the lock is held by
//               some other thread.
////////////////////////////////////////////////////////////////////
INLINE bool ReMutex::
debug_is_locked() const {
  MutexHolder holder(_mutex);
  return (_locking_thread == Thread::get_current_thread());
}

#else  // !THREAD_DUMMY_IMPL || CHECK_REENTRANT_MUTEX

// In this branch, the mutex is stubbed out to do nothing.

////////////////////////////////////////////////////////////////////
//     Function: ReMutex::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ReMutex::
ReMutex() {
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutex::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ReMutex::
~ReMutex() {
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutex::Copy Constructor
//       Access: Private
//  Description: Do not attempt to copy mutexes.
////////////////////////////////////////////////////////////////////
INLINE ReMutex::
ReMutex(const ReMutex &copy) {
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
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutex::debug_is_locked
//       Access: Public
//  Description: Returns true if the current thread has locked the
//               Mutex, false otherwise.  This method is only
//               meaningful if CHECK_REENTRANT_MUTEX is defined;
//               otherwise, it always returns true.
////////////////////////////////////////////////////////////////////
INLINE bool ReMutex::
debug_is_locked() const {
  return true;
}

#endif  // !THREAD_DUMMY_IMPL || CHECK_REENTRANT_MUTEX

