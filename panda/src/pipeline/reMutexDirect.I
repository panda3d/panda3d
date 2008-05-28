// Filename: reMutexDirect.I
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
//     Function: ReMutexDirect::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ReMutexDirect::
ReMutexDirect() 
#ifndef HAVE_REMUTEXIMPL
  : _cvar_impl(_lock_impl)
#endif
{
#ifndef HAVE_REMUTEXIMPL
  _locking_thread = NULL;
  _lock_count = 0;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::Destructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ReMutexDirect::
~ReMutexDirect() {
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::Copy Constructor
//       Access: Private
//  Description: Do not attempt to copy reMutexes.
////////////////////////////////////////////////////////////////////
INLINE ReMutexDirect::
ReMutexDirect(const ReMutexDirect &copy)
#ifndef HAVE_REMUTEXIMPL
  : _cvar_impl(_lock_impl)
#endif
{
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::Copy Assignment Operator
//       Access: Private
//  Description: Do not attempt to copy reMutexes.
////////////////////////////////////////////////////////////////////
INLINE void ReMutexDirect::
operator = (const ReMutexDirect &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::lock
//       Access: Published
//  Description: Grabs the reMutex if it is available.  If it is not
//               available, blocks until it becomes available, then
//               grabs it.  In either case, the function does not
//               return until the reMutex is held; you should then call
//               unlock().
//
//               This method is considered const so that you can lock
//               and unlock const reMutexes, mainly to allow thread-safe
//               access to otherwise const data.
//
//               Also see ReMutexHolder.
////////////////////////////////////////////////////////////////////
INLINE void ReMutexDirect::
lock() const {
  TAU_PROFILE("void ReMutexDirect::lock()", " ", TAU_USER);
#ifdef HAVE_REMUTEXIMPL
  ((ReMutexDirect *)this)->_impl.lock();
#else
  ((ReMutexDirect *)this)->do_lock();
#endif  // HAVE_REMUTEXIMPL
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::lock
//       Access: Published
//  Description: This variant on lock() accepts the current thread as
//               a parameter, if it is already known, as an
//               optimization.
////////////////////////////////////////////////////////////////////
INLINE void ReMutexDirect::
lock(Thread *current_thread) const {
  TAU_PROFILE("void ReMutexDirect::lock(Thread *)", " ", TAU_USER);
#ifdef HAVE_REMUTEXIMPL
  ((ReMutexDirect *)this)->_impl.lock();
#else
  ((ReMutexDirect *)this)->do_lock(current_thread);
#endif  // HAVE_REMUTEXIMPL
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::elevate_lock
//       Access: Published
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
INLINE void ReMutexDirect::
elevate_lock() const {
  TAU_PROFILE("void ReMutexDirect::elevate_lock()", " ", TAU_USER);
#ifdef HAVE_REMUTEXIMPL
  ((ReMutexDirect *)this)->_impl.lock();
#else
  ((ReMutexDirect *)this)->do_elevate_lock();
#endif  // HAVE_REMUTEXIMPL
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::release
//       Access: Published
//  Description: Releases the reMutex.  It is an error to call this if
//               the reMutex was not already locked.
//
//               This method is considered const so that you can lock
//               and unlock const reMutexes, mainly to allow thread-safe
//               access to otherwise const data.
////////////////////////////////////////////////////////////////////
INLINE void ReMutexDirect::
release() const {
  TAU_PROFILE("void ReMutexDirect::release()", " ", TAU_USER);
#ifdef HAVE_REMUTEXIMPL
  ((ReMutexDirect *)this)->_impl.release();
#else
  ((ReMutexDirect *)this)->do_release();
#endif  // HAVE_REMUTEXIMPL
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::debug_is_locked
//       Access: Published
//  Description: Returns true if the current thread has locked the
//               ReMutex, false otherwise.  This method is only intended
//               for use in debugging, hence the method name; in the
//               ReMutexDirect case, it always returns true, since
//               there's not a reliable way to determine this
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool ReMutexDirect::
debug_is_locked() const {
  return true;
}

#ifndef HAVE_REMUTEXIMPL
////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::do_lock
//       Access: Private
//  Description: The private implementation of lock(), for the case in
//               which the underlying lock system does not provide a
//               reentrant mutex (and therefore we have to build this
//               functionality on top of the existing non-reentrant
//               mutex).
////////////////////////////////////////////////////////////////////
INLINE void ReMutexDirect::
do_lock() {
  do_lock(Thread::get_current_thread());
}
#endif

