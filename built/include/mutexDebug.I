/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mutexDebug.I
 * @author drose
 * @date 2006-02-13
 */

/**
 * Alias for acquire() to match C++11 semantics.
 * @see acquire()
 */
INLINE void MutexDebug::
lock() {
  TAU_PROFILE("void MutexDebug::acquire()", " ", TAU_USER);
  _global_lock->lock();
  ((MutexDebug *)this)->do_lock(Thread::get_current_thread());
  _global_lock->unlock();
}

/**
 * Alias for try_acquire() to match C++11 semantics.
 * @see try_acquire()
 */
INLINE bool MutexDebug::
try_lock() {
  TAU_PROFILE("void MutexDebug::try_lock()", " ", TAU_USER);
  _global_lock->lock();
  bool acquired = ((MutexDebug *)this)->do_try_lock(Thread::get_current_thread());
  _global_lock->unlock();
  return acquired;
}

/**
 * Alias for release() to match C++11 semantics.
 * @see release()
 */
INLINE void MutexDebug::
unlock() {
  TAU_PROFILE("void MutexDebug::unlock()", " ", TAU_USER);
  _global_lock->lock();
  ((MutexDebug *)this)->do_unlock();
  _global_lock->unlock();
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
INLINE void MutexDebug::
acquire(Thread *current_thread) const {
  TAU_PROFILE("void MutexDebug::acquire(Thread *)", " ", TAU_USER);
  nassertv(current_thread == Thread::get_current_thread());
  _global_lock->lock();
  ((MutexDebug *)this)->do_lock(current_thread);
  _global_lock->unlock();
}

/**
 * Returns immediately, with a true value indicating the mutex has been
 * acquired, and false indicating it has not.
 *
 * @deprecated Python users should use acquire(False), C++ users try_lock()
 */
INLINE bool MutexDebug::
try_acquire(Thread *current_thread) const {
  TAU_PROFILE("void MutexDebug::try_acquire(Thread *)", " ", TAU_USER);
  nassertr(current_thread == Thread::get_current_thread(), false);
  _global_lock->lock();
  bool acquired = ((MutexDebug *)this)->do_try_lock(current_thread);
  _global_lock->unlock();
  return acquired;
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
INLINE void MutexDebug::
elevate_lock() const {
  TAU_PROFILE("void MutexDebug::elevate_lock()", " ", TAU_USER);
  // You may only pass call elevate_lock() on a ReMutex--that is, to a mutex
  // whose _allow_recursion flag is true.
  nassertv(_allow_recursion);

  // Also, it's an error to call this if the lock is not already held.
  nassertv(debug_is_locked());

  acquire();
}

/**
 * Releases the mutex.  It is an error to call this if the mutex was not
 * already locked.
 *
 * This method is considered const so that you can lock and unlock const
 * mutexes, mainly to allow thread-safe access to otherwise const data.
 */
INLINE void MutexDebug::
release() const {
  TAU_PROFILE("void MutexDebug::release()", " ", TAU_USER);
  _global_lock->lock();
  ((MutexDebug *)this)->do_unlock();
  _global_lock->unlock();
}

/**
 * Returns true if the current thread has locked the Mutex, false otherwise.
 * This method is only intended for use in debugging, hence the method name;
 * in the MutexDebug case, it always returns true, since there's not a
 * reliable way to determine this otherwise.
 */
INLINE bool MutexDebug::
debug_is_locked() const {
  TAU_PROFILE("bool MutexDebug::debug_is_locked()", " ", TAU_USER);
  _global_lock->lock();
  bool is_locked = do_debug_is_locked();
  _global_lock->unlock();
  return is_locked;
}

/**
 * Ensures the global MutexImpl pointer has been created, and returns its
 * pointer.  Since this method is called by the MutexDebug constructor, any
 * other (non-static) methods of MutexDebug may simply assume that the pointer
 * has already been created.
 */
INLINE MutexTrueImpl *MutexDebug::
get_global_lock() {
  if (_global_lock == nullptr) {
    _global_lock = new MutexTrueImpl;
  }
  return _global_lock;
}
