/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file reMutexDirect.I
 * @author drose
 * @date 2006-02-13
 */

/**
 *
 */
INLINE ReMutexDirect::
ReMutexDirect()
#ifndef HAVE_REMUTEXTRUEIMPL
  : _cvar_impl(_lock_impl)
#endif
{
#ifndef HAVE_REMUTEXTRUEIMPL
  _locking_thread = nullptr;
  _lock_count = 0;
#endif
}

/**
 * Alias for acquire() to match C++11 semantics.
 * @see acquire()
 */
INLINE void ReMutexDirect::
lock() {
  TAU_PROFILE("void ReMutexDirect::acquire()", " ", TAU_USER);
#ifdef HAVE_REMUTEXTRUEIMPL
  _impl.lock();
#else
  ((ReMutexDirect *)this)->do_lock();
#endif  // HAVE_REMUTEXTRUEIMPL
}

/**
 * Alias for try_acquire() to match C++11 semantics.
 * @see try_acquire()
 */
INLINE bool ReMutexDirect::
try_lock() {
  TAU_PROFILE("void ReMutexDirect::try_acquire()", " ", TAU_USER);
#ifdef HAVE_REMUTEXTRUEIMPL
  return _impl.try_lock();
#else
  return ((ReMutexDirect *)this)->do_try_lock();
#endif  // HAVE_REMUTEXTRUEIMPL
}

/**
 * Alias for release() to match C++11 semantics.
 * @see release()
 */
INLINE void ReMutexDirect::
unlock() {
  TAU_PROFILE("void ReMutexDirect::unlock()", " ", TAU_USER);
#ifdef HAVE_REMUTEXTRUEIMPL
  _impl.unlock();
#else
  ((ReMutexDirect *)this)->do_unlock();
#endif  // HAVE_REMUTEXTRUEIMPL
}

/**
 * Grabs the reMutex if it is available.  If it is not available, blocks until
 * it becomes available, then grabs it.  In either case, the function does not
 * return until the reMutex is held; you should then call unlock().
 *
 * This method is considered const so that you can lock and unlock const
 * reMutexes, mainly to allow thread-safe access to otherwise const data.
 *
 * Also see ReMutexHolder.
 */
INLINE void ReMutexDirect::
acquire() const {
  TAU_PROFILE("void ReMutexDirect::acquire()", " ", TAU_USER);
#ifdef HAVE_REMUTEXTRUEIMPL
  _impl.lock();
#else
  ((ReMutexDirect *)this)->do_lock();
#endif  // HAVE_REMUTEXTRUEIMPL
}

/**
 * This variant on acquire() accepts the current thread as a parameter, if it
 * is already known, as an optimization.
 */
INLINE void ReMutexDirect::
acquire(Thread *current_thread) const {
  TAU_PROFILE("void ReMutexDirect::acquire(Thread *)", " ", TAU_USER);
#ifdef HAVE_REMUTEXTRUEIMPL
  _impl.lock();
#else
  ((ReMutexDirect *)this)->do_lock(current_thread);
#endif  // HAVE_REMUTEXTRUEIMPL
}

/**
 * Returns immediately, with a true value indicating the mutex has been
 * acquired, and false indicating it has not.
 *
 * @deprecated Python users should use acquire(False), C++ users try_lock()
 */
INLINE bool ReMutexDirect::
try_acquire() const {
  TAU_PROFILE("void ReMutexDirect::acquire(bool)", " ", TAU_USER);
#ifdef HAVE_REMUTEXTRUEIMPL
  return _impl.try_lock();
#else
  return ((ReMutexDirect *)this)->do_try_lock();
#endif  // HAVE_REMUTEXTRUEIMPL
}

/**
 * Returns immediately, with a true value indicating the mutex has been
 * acquired, and false indicating it has not.
 *
 * @deprecated Python users should use acquire(False), C++ users try_lock()
 */
INLINE bool ReMutexDirect::
try_acquire(Thread *current_thread) const {
  TAU_PROFILE("void ReMutexDirect::acquire(bool)", " ", TAU_USER);
#ifdef HAVE_REMUTEXTRUEIMPL
  return _impl.try_lock();
#else
  return ((ReMutexDirect *)this)->do_try_lock(current_thread);
#endif  // HAVE_REMUTEXTRUEIMPL
}

/**
 * This method increments the lock count, assuming the calling thread already
 * holds the lock.  After this call, release() will need to be called one
 * additional time to release the lock.
 *
 * This method really performs the same function as acquire(), but it offers a
 * potential (slight) performance benefit when the calling thread knows that
 * it already holds the lock.  It is an error to call this when the calling
 * thread does not hold the lock.
 */
INLINE void ReMutexDirect::
elevate_lock() const {
  TAU_PROFILE("void ReMutexDirect::elevate_lock()", " ", TAU_USER);
#ifdef HAVE_REMUTEXTRUEIMPL
  _impl.lock();
#else
  ((ReMutexDirect *)this)->do_elevate_lock();
#endif  // HAVE_REMUTEXTRUEIMPL
}

/**
 * Releases the reMutex.  It is an error to call this if the reMutex was not
 * already locked.
 *
 * This method is considered const so that you can lock and unlock const
 * reMutexes, mainly to allow thread-safe access to otherwise const data.
 */
INLINE void ReMutexDirect::
release() const {
  TAU_PROFILE("void ReMutexDirect::release()", " ", TAU_USER);
#ifdef HAVE_REMUTEXTRUEIMPL
  _impl.unlock();
#else
  ((ReMutexDirect *)this)->do_unlock();
#endif  // HAVE_REMUTEXTRUEIMPL
}

/**
 * Returns true if the current thread has locked the ReMutex, false otherwise.
 * This method is only intended for use in debugging, hence the method name;
 * in the ReMutexDirect case, it always returns true, since there's not a
 * reliable way to determine this otherwise.
 */
INLINE bool ReMutexDirect::
debug_is_locked() const {
  return true;
}

/**
 * The mutex name is only defined when compiling in DEBUG_THREADS mode.
 */
INLINE void ReMutexDirect::
set_name(const std::string &) {
}

/**
 * The mutex name is only defined when compiling in DEBUG_THREADS mode.
 */
INLINE void ReMutexDirect::
clear_name() {
}

/**
 * The mutex name is only defined when compiling in DEBUG_THREADS mode.
 */
INLINE bool ReMutexDirect::
has_name() const {
  return false;
}

/**
 * The mutex name is only defined when compiling in DEBUG_THREADS mode.
 */
INLINE std::string ReMutexDirect::
get_name() const {
  return std::string();
}

#ifndef HAVE_REMUTEXTRUEIMPL
/**
 * The private implementation of acquire(), for the case in which the
 * underlying lock system does not provide a reentrant mutex (and therefore we
 * have to build this functionality on top of the existing non-reentrant
 * mutex).
 */
INLINE void ReMutexDirect::
do_lock() {
  do_lock(Thread::get_current_thread());
}
#endif

#ifndef HAVE_REMUTEXTRUEIMPL
/**
 * The private implementation of acquire(false), for the case in which the
 * underlying lock system does not provide a reentrant mutex (and therefore we
 * have to build this functionality on top of the existing non-reentrant
 * mutex).
 */
INLINE bool ReMutexDirect::
do_try_lock() {
  return do_try_lock(Thread::get_current_thread());
}
#endif
