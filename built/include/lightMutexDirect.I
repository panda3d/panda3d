/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightMutexDirect.I
 * @author drose
 * @date 2008-10-08
 */

/**
 * Alias for acquire() to match C++11 semantics.
 * @see acquire()
 */
INLINE void LightMutexDirect::
lock() {
  TAU_PROFILE("void LightMutexDirect::acquire()", " ", TAU_USER);
  _impl.lock();
}

/**
 * Alias for try_acquire() to match C++11 semantics.
 * @see try_acquire()
 */
INLINE bool LightMutexDirect::
try_lock() {
  TAU_PROFILE("void LightMutexDirect::try_acquire()", " ", TAU_USER);
  return _impl.try_lock();
}

/**
 * Alias for release() to match C++11 semantics.
 * @see release()
 */
INLINE void LightMutexDirect::
unlock() {
  TAU_PROFILE("void LightMutexDirect::unlock()", " ", TAU_USER);
  _impl.unlock();
}

/**
 * Grabs the lightMutex if it is available.  If it is not available, blocks
 * until it becomes available, then grabs it.  In either case, the function
 * does not return until the lightMutex is held; you should then call
 * unlock().
 *
 * This method is considered const so that you can lock and unlock const
 * lightMutexes, mainly to allow thread-safe access to otherwise const data.
 *
 * Also see LightMutexHolder.
 */
INLINE void LightMutexDirect::
acquire() const {
  TAU_PROFILE("void LightMutexDirect::acquire()", " ", TAU_USER);
  _impl.lock();
}

/**
 * Releases the lightMutex.  It is an error to call this if the lightMutex was
 * not already locked.
 *
 * This method is considered const so that you can lock and unlock const
 * lightMutexes, mainly to allow thread-safe access to otherwise const data.
 */
INLINE void LightMutexDirect::
release() const {
  TAU_PROFILE("void LightMutexDirect::release()", " ", TAU_USER);
  _impl.unlock();
}

/**
 * Returns true if the current thread has locked the LightMutex, false
 * otherwise.  This method is only intended for use in debugging, hence the
 * method name; in the LightMutexDirect case, it always returns true, since
 * there's not a reliable way to determine this otherwise.
 */
INLINE bool LightMutexDirect::
debug_is_locked() const {
  return true;
}

/**
 * The lightMutex name is only defined when compiling in DEBUG_THREADS mode.
 */
INLINE void LightMutexDirect::
set_name(const std::string &) {
}

/**
 * The lightMutex name is only defined when compiling in DEBUG_THREADS mode.
 */
INLINE void LightMutexDirect::
clear_name() {
}

/**
 * The lightMutex name is only defined when compiling in DEBUG_THREADS mode.
 */
INLINE bool LightMutexDirect::
has_name() const {
  return false;
}

/**
 * The lightMutex name is only defined when compiling in DEBUG_THREADS mode.
 */
INLINE std::string LightMutexDirect::
get_name() const {
  return std::string();
}
