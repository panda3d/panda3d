/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightMutexHolder.I
 * @author drose
 * @date 2008-10-08
 */

/**
 *
 */
INLINE LightMutexHolder::
LightMutexHolder(const LightMutex &mutex) {
#if defined(HAVE_THREADS) || defined(DEBUG_THREADS)
  _mutex = &mutex;
  _mutex->acquire();
#endif
}

/**
 * If the LightMutexHolder constructor is given a pointer to a LightMutex
 * object (instead of an actual object), it will first check to see if the
 * pointer is NULL, and allocate a new LightMutex if it is.  This is intended
 * as a convenience for functions that may need to reference a LightMutex at
 * static init time, when it is impossible to guarantee ordering of
 * initializers.
 */
INLINE LightMutexHolder::
LightMutexHolder(LightMutex *&mutex) {
#if defined(HAVE_THREADS) || defined(DEBUG_THREADS)
  if (mutex == nullptr) {
    mutex = new LightMutex;
  }
  _mutex = mutex;
  _mutex->acquire();
#endif
}

/**
 *
 */
INLINE LightMutexHolder::
~LightMutexHolder() {
#if defined(HAVE_THREADS) || defined(DEBUG_THREADS)
  _mutex->release();
#endif
}
