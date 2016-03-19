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
 *
 */
INLINE LightReMutexDirect::
LightReMutexDirect()
#ifndef HAVE_REMUTEXIMPL
  : _cvar_impl(_lock_impl)
#endif
{
#ifndef HAVE_REMUTEXIMPL
  _locking_thread = NULL;
  _lock_count = 0;
#endif
}

/**
 *
 */
INLINE LightReMutexDirect::
~LightReMutexDirect() {
}

/**
 * Do not attempt to copy lightReMutexes.
 */
INLINE LightReMutexDirect::
LightReMutexDirect(const LightReMutexDirect &copy)
#ifndef HAVE_REMUTEXIMPL
  : _cvar_impl(_lock_impl)
#endif
{
  nassertv(false);
}

/**
 * Do not attempt to copy lightReMutexes.
 */
INLINE void LightReMutexDirect::
operator = (const LightReMutexDirect &copy) {
  nassertv(false);
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
  ((LightReMutexDirect *)this)->_impl.acquire();
}

/**
 * This variant on acquire() accepts the current thread as a parameter, if it
 * is already known, as an optimization.
 */
INLINE void LightReMutexDirect::
acquire(Thread *current_thread) const {
  TAU_PROFILE("void LightReMutexDirect::acquire(Thread *)", " ", TAU_USER);
#ifdef HAVE_REMUTEXIMPL
  ((LightReMutexDirect *)this)->_impl.acquire();
#else
  ((LightReMutexDirect *)this)->_impl.do_lock(current_thread);
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
#ifdef HAVE_REMUTEXIMPL
  ((LightReMutexDirect *)this)->_impl.acquire();
#else
  ((LightReMutexDirect *)this)->_impl.do_elevate_lock();
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
  ((LightReMutexDirect *)this)->_impl.release();
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
set_name(const string &) {
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
INLINE string LightReMutexDirect::
get_name() const {
  return string();
}
