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
#ifndef HAVE_REMUTEXTRUEIMPL
  : _cvar_impl(_lock_impl)
#endif
{
#ifndef HAVE_REMUTEXTRUEIMPL
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
#ifndef HAVE_REMUTEXTRUEIMPL
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
//     Function: ReMutexDirect::acquire
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
acquire() const {
  TAU_PROFILE("void ReMutexDirect::acquire()", " ", TAU_USER);
#ifdef HAVE_REMUTEXTRUEIMPL
  ((ReMutexDirect *)this)->_impl.acquire();
#else
  ((ReMutexDirect *)this)->do_acquire();
#endif  // HAVE_REMUTEXTRUEIMPL
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::acquire
//       Access: Published
//  Description: This variant on acquire() accepts the current thread as
//               a parameter, if it is already known, as an
//               optimization.
////////////////////////////////////////////////////////////////////
INLINE void ReMutexDirect::
acquire(Thread *current_thread) const {
  TAU_PROFILE("void ReMutexDirect::acquire(Thread *)", " ", TAU_USER);
#ifdef HAVE_REMUTEXTRUEIMPL
  ((ReMutexDirect *)this)->_impl.acquire();
#else
  ((ReMutexDirect *)this)->do_acquire(current_thread);
#endif  // HAVE_REMUTEXTRUEIMPL
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::try_acquire
//       Access: Published
//  Description: Returns immediately, with a true value indicating the
//               mutex has been acquired, and false indicating it has
//               not.
////////////////////////////////////////////////////////////////////
INLINE bool ReMutexDirect::
try_acquire() const {
  TAU_PROFILE("void ReMutexDirect::acquire(bool)", " ", TAU_USER);
#ifdef HAVE_REMUTEXTRUEIMPL
  return ((ReMutexDirect *)this)->_impl.try_acquire();
#else
  return ((ReMutexDirect *)this)->do_try_acquire();
#endif  // HAVE_REMUTEXTRUEIMPL
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::try_acquire
//       Access: Published
//  Description: Returns immediately, with a true value indicating the
//               mutex has been acquired, and false indicating it has
//               not.
////////////////////////////////////////////////////////////////////
INLINE bool ReMutexDirect::
try_acquire(Thread *current_thread) const {
  TAU_PROFILE("void ReMutexDirect::acquire(bool)", " ", TAU_USER);
#ifdef HAVE_REMUTEXTRUEIMPL
  return ((ReMutexDirect *)this)->_impl.try_acquire();
#else
  return ((ReMutexDirect *)this)->do_try_acquire(current_thread);
#endif  // HAVE_REMUTEXTRUEIMPL
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
//               acquire(), but it offers a potential (slight)
//               performance benefit when the calling thread knows
//               that it already holds the lock.  It is an error to
//               call this when the calling thread does not hold the
//               lock.
////////////////////////////////////////////////////////////////////
INLINE void ReMutexDirect::
elevate_lock() const {
  TAU_PROFILE("void ReMutexDirect::elevate_lock()", " ", TAU_USER);
#ifdef HAVE_REMUTEXTRUEIMPL
  ((ReMutexDirect *)this)->_impl.acquire();
#else
  ((ReMutexDirect *)this)->do_elevate_lock();
#endif  // HAVE_REMUTEXTRUEIMPL
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
#ifdef HAVE_REMUTEXTRUEIMPL
  ((ReMutexDirect *)this)->_impl.release();
#else
  ((ReMutexDirect *)this)->do_release();
#endif  // HAVE_REMUTEXTRUEIMPL
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

////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::set_name
//       Access: Public
//  Description: The mutex name is only defined when compiling in
//               DEBUG_THREADS mode.
////////////////////////////////////////////////////////////////////
INLINE void ReMutexDirect::
set_name(const string &) {
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::clear_name
//       Access: Public
//  Description: The mutex name is only defined when compiling in
//               DEBUG_THREADS mode.
////////////////////////////////////////////////////////////////////
INLINE void ReMutexDirect::
clear_name() {
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::has_name
//       Access: Public
//  Description: The mutex name is only defined when compiling in
//               DEBUG_THREADS mode.
////////////////////////////////////////////////////////////////////
INLINE bool ReMutexDirect::
has_name() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::get_name
//       Access: Public
//  Description: The mutex name is only defined when compiling in
//               DEBUG_THREADS mode.
////////////////////////////////////////////////////////////////////
INLINE string ReMutexDirect::
get_name() const {
  return string();
}

#ifndef HAVE_REMUTEXTRUEIMPL
////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::do_acquire
//       Access: Private
//  Description: The private implementation of acquire(), for the case in
//               which the underlying lock system does not provide a
//               reentrant mutex (and therefore we have to build this
//               functionality on top of the existing non-reentrant
//               mutex).
////////////////////////////////////////////////////////////////////
INLINE void ReMutexDirect::
do_acquire() {
  do_acquire(Thread::get_current_thread());
}
#endif

#ifndef HAVE_REMUTEXTRUEIMPL
////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::do_try_acquire
//       Access: Private
//  Description: The private implementation of acquire(false), for the
//               case in which the underlying lock system does not
//               provide a reentrant mutex (and therefore we have to
//               build this functionality on top of the existing
//               non-reentrant mutex).
////////////////////////////////////////////////////////////////////
INLINE bool ReMutexDirect::
do_try_acquire() {
  return do_try_acquire(Thread::get_current_thread());
}
#endif

