/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pmutex_ext.h
 * @author rdb
 * @date 2019-05-12
 */

/**
 * Acquires the mutex.
 */
INLINE bool Extension<Mutex>::
acquire(bool blocking) const {
  if (_this->try_lock()) {
    return true;
  }

  if (!blocking) {
    return false;
  }

  // Release the GIL while we are waiting for the lock.
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyThreadState *_save;
  Py_UNBLOCK_THREADS
  _this->lock();
  Py_BLOCK_THREADS
#else
  _this->lock();
#endif
  return true;
}

/**
 * Acquires the mutex.
 */
INLINE bool Extension<Mutex>::
__enter__() {
  return acquire(true);
}

/**
 * Releases the mutex.
 */
INLINE void Extension<Mutex>::
__exit__(PyObject *, PyObject *, PyObject *) {
  _this->unlock();
}
