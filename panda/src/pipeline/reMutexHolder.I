/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file reMutexHolder.I
 * @author drose
 * @date 2006-01-15
 */

/**
 *
 */
INLINE ReMutexHolder::
ReMutexHolder(const ReMutex &mutex) {
#if defined(HAVE_THREADS) || defined(DEBUG_THREADS)
  _mutex = &mutex;
  _mutex->acquire();
#endif
}

/**
 * This variant on the constructor accepts the current thread as a parameter,
 * if it is already known, as an optimization.
 */
INLINE ReMutexHolder::
ReMutexHolder(const ReMutex &mutex, Thread *current_thread) {
#if defined(HAVE_THREADS) || defined(DEBUG_THREADS)
  _mutex = &mutex;
  _mutex->acquire(current_thread);
#endif
}

/**
 * If the ReMutexHolder constructor is given a pointer to a ReMutex object
 * (instead of an actual object), it will first check to see if the pointer is
 * NULL, and allocate a new ReMutex if it is.  This is intended as a
 * convenience for functions that may need to reference a ReMutex at static
 * init time, when it is impossible to guarantee ordering of initializers.
 */
INLINE ReMutexHolder::
ReMutexHolder(ReMutex *&mutex) {
#if defined(HAVE_THREADS) || defined(DEBUG_THREADS)
  if (mutex == nullptr) {
    mutex = new ReMutex;
  }
  _mutex = mutex;
  _mutex->acquire();
#endif
}

/**
 *
 */
INLINE ReMutexHolder::
~ReMutexHolder() {
#if defined(HAVE_THREADS) || defined(DEBUG_THREADS)
  _mutex->release();
#endif
}
