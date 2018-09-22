/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightReMutexDirect.I
 * @author drose
 * @date 2008-10-08
 */

/**
 * Alias for acquire() to match C++11 semantics.
 * @see acquire()
 */
INLINE void LightReMutexDirect::
lock() {
  TAU_PROFILE("void LightReMutexDirect::acquire()", " ", TAU_USER);
  _impl.lock();
}

/**
 * Alias for try_acquire() to match C++11 semantics.
 * @see try_acquire()
 */
INLINE bool LightReMutexDirect::
try_lock() {
  TAU_PROFILE("void LightReMutexDirect::try_acquire()", " ", TAU_USER);
  return _impl.try_lock();
}

/**
 * Alias for release() to match C++11 semantics.
 * @see release()
 */
INLINE void LightReMutexDirect::
unlock() {
  TAU_PROFILE("void LightReMutexDirect::unlock()", " ", TAU_USER);
  _impl.unlock();
}

/**
 * Grabs the lightReMutex if it is available.  If it is not available, blocks
 * until it becomes available, then grabs it.  In either case, the function
 * does not return until the lightReMutex is held; you should then call
 * unlock().
 *
 * This method is considered const so that you can lock and unlock const
 * lightReMutexes, mainly to allow thread-safe access to otherwise const data.
 *
 * Also see LightReMutexHolder.
 */
INLINE void LightReMutexDirect::
acquire() const {
  TAU_PROFILE("void LightReMutexDirect::acquire()", " ", TAU_USER);
#ifdef HAVE_REMUTEXTRUEIMPL
  _impl.lock();
#else
  _impl.do_lock(Thread::get_current_thread());
#endif
}

/**
 * This variant on acquire() accepts the current thread as a parameter, if it
 * is already known, as an optimization.
 */
INLINE void LightReMutexDirect::
acquire(Thread *current_thread) const {
  TAU_PROFILE("void LightReMutexDirect::acquire(Thread *)", " ", TAU_USER);
#ifdef HAVE_REMUTEXTRUEIMPL
  _impl.lock();
#else
  _impl.do_lock(current_thread);
#endif  // HAVE_REMUTEXIMPL
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
INLINE void LightReMutexDirect::
elevate_lock() const {
  TAU_PROFILE("void LightReMutexDirect::elevate_lock()", " ", TAU_USER);
#ifdef HAVE_REMUTEXTRUEIMPL
  _impl.lock();
#else
  _impl.do_elevate_lock();
#endif  // HAVE_REMUTEXIMPL
}

/**
 * Releases the lightReMutex.  It is an error to call this if the lightReMutex
 * was not already locked.
 *
 * This method is considered const so that you can lock and unlock const
 * lightReMutexes, mainly to allow thread-safe access to otherwise const data.
 */
INLINE void LightReMutexDirect::
release() const {
  TAU_PROFILE("void LightReMutexDirect::release()", " ", TAU_USER);
#ifdef HAVE_REMUTEXTRUEIMPL
  _impl.unlock();
#else
  _impl.do_unlock(Thread::get_current_thread());
#endif
}

/**
 * Returns true if the current thread has locked the LightReMutex, false
 * otherwise.  This method is only intended for use in debugging, hence the
 * method name; in the LightReMutexDirect case, it always returns true, since
 * there's not a reliable way to determine this otherwise.
 */
INLINE bool LightReMutexDirect::
debug_is_locked() const {
  return true;
}

/**
 * The mutex name is only defined when compiling in DEBUG_THREADS mode.
 */
INLINE void LightReMutexDirect::
set_name(const std::string &) {
}

/**
 * The mutex name is only defined when compiling in DEBUG_THREADS mode.
 */
INLINE void LightReMutexDirect::
clear_name() {
}

/**
 * The mutex name is only defined when compiling in DEBUG_THREADS mode.
 */
INLINE bool LightReMutexDirect::
has_name() const {
  return false;
}

/**
 * The mutex name is only defined when compiling in DEBUG_THREADS mode.
 */
INLINE std::string LightReMutexDirect::
get_name() const {
  return std::string();
}
