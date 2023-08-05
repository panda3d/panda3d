/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mutexDirect.I
 * @author drose
 * @date 2006-02-13
 */

/**
 * Alias for acquire() to match C++11 semantics.
 * @see acquire()
 */
INLINE void MutexDirect::
lock() {
  TAU_PROFILE("void MutexDirect::acquire()", " ", TAU_USER);
  _impl.lock();
}

/**
 * Alias for try_acquire() to match C++11 semantics.
 * @see try_acquire()
 */
INLINE bool MutexDirect::
try_lock() {
  TAU_PROFILE("void MutexDirect::try_acquire()", " ", TAU_USER);
  return _impl.try_lock();
}

/**
 * Alias for release() to match C++11 semantics.
 * @see release()
 */
INLINE void MutexDirect::
unlock() {
  TAU_PROFILE("void MutexDirect::unlock()", " ", TAU_USER);
  _impl.unlock();
}

/**
 * Grabs the mutex if it is available.  If it is not available, blocks until
 * it becomes available, then grabs it.  In either case, the function does not
 * return until the mutex is held; you should then call unlock().
 *
 * This method is considered const so that you can lock and unlock const
 * mutexes, mainly to allow thread-safe access to otherwise const data.
 *
 * Also see MutexHolder.
 */
INLINE void MutexDirect::
acquire() const {
  TAU_PROFILE("void MutexDirect::acquire()", " ", TAU_USER);
  _impl.lock();
}

/**
 * Returns immediately, with a true value indicating the mutex has been
 * acquired, and false indicating it has not.
 *
 * @deprecated Python users should use acquire(False), C++ users try_lock()
 */
INLINE bool MutexDirect::
try_acquire() const {
  TAU_PROFILE("void MutexDirect::acquire(bool)", " ", TAU_USER);
  return _impl.try_lock();
}

/**
 * Releases the mutex.  It is an error to call this if the mutex was not
 * already locked.
 *
 * This method is considered const so that you can lock and unlock const
 * mutexes, mainly to allow thread-safe access to otherwise const data.
 */
INLINE void MutexDirect::
release() const {
  TAU_PROFILE("void MutexDirect::release()", " ", TAU_USER);
  _impl.unlock();
}

/**
 * Returns true if the current thread has locked the Mutex, false otherwise.
 * This method is only intended for use in debugging, hence the method name;
 * in the MutexDirect case, it always returns true, since there's not a
 * reliable way to determine this otherwise.
 */
INLINE bool MutexDirect::
debug_is_locked() const {
  return true;
}

/**
 * The mutex name is only defined when compiling in DEBUG_THREADS mode.
 */
INLINE void MutexDirect::
set_name(const std::string &) {
}

/**
 * The mutex name is only defined when compiling in DEBUG_THREADS mode.
 */
INLINE void MutexDirect::
clear_name() {
}

/**
 * The mutex name is only defined when compiling in DEBUG_THREADS mode.
 */
INLINE bool MutexDirect::
has_name() const {
  return false;
}

/**
 * The mutex name is only defined when compiling in DEBUG_THREADS mode.
 */
INLINE std::string MutexDirect::
get_name() const {
  return std::string();
}
