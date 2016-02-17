/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mutexPosixImpl.I
 * @author drose
 * @date 2006-02-10
 */

/**

 */
INLINE MutexPosixImpl::
MutexPosixImpl() {
  TAU_PROFILE("MutexPosixImpl::MutexPosixImpl", " ", TAU_USER);
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  // The symbol PTHREAD_MUTEX_DEFAULT isn't always available?
  // pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_DEFAULT);
  int result = pthread_mutex_init(&_lock, &attr);
  pthread_mutexattr_destroy(&attr);
  assert(result == 0);
}

/**

 */
INLINE MutexPosixImpl::
~MutexPosixImpl() {
  TAU_PROFILE("MutexPosixImpl::~MutexPosixImpl", " ", TAU_USER);
  int result = pthread_mutex_destroy(&_lock);
  assert(result == 0);
}

/**

 */
INLINE void MutexPosixImpl::
acquire() {
  TAU_PROFILE("void MutexPosixImpl::acquire", " ", TAU_USER);
  int result = pthread_mutex_lock(&_lock);
  assert(result == 0);
}

/**

 */
INLINE bool MutexPosixImpl::
try_acquire() {
  TAU_PROFILE("bool MutexPosixImpl::try_acquire", " ", TAU_USER);
  int result = pthread_mutex_trylock(&_lock);
  assert(result == 0 || result == EBUSY);
  return (result == 0);
}

/**

 */
INLINE void MutexPosixImpl::
release() {
  TAU_PROFILE("void MutexPosixImpl::release", " ", TAU_USER);
  int result = pthread_mutex_unlock(&_lock);
  assert(result == 0);
}

/**
 * Returns the underlying Posix lock handle.
 */
INLINE pthread_mutex_t *MutexPosixImpl::
get_posix_lock() {
  return &_lock;
}

/**

 */
INLINE ReMutexPosixImpl::
ReMutexPosixImpl() {
  TAU_PROFILE("ReMutexPosixImpl::ReMutexPosixImpl", " ", TAU_USER);
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
  int result = pthread_mutex_init(&_lock, &attr);
  pthread_mutexattr_destroy(&attr);
  assert(result == 0);
}

/**

 */
INLINE ReMutexPosixImpl::
~ReMutexPosixImpl() {
  TAU_PROFILE("ReMutexPosixImpl::~ReMutexPosixImpl", " ", TAU_USER);
  int result = pthread_mutex_destroy(&_lock);
  assert(result == 0);
}

/**

 */
INLINE void ReMutexPosixImpl::
acquire() {
  TAU_PROFILE("void ReMutexPosixImpl::acquire", " ", TAU_USER);
  int result = pthread_mutex_lock(&_lock);
  assert(result == 0);
}

/**

 */
INLINE bool ReMutexPosixImpl::
try_acquire() {
  TAU_PROFILE("bool ReMutexPosixImpl::try_acquire", " ", TAU_USER);
  int result = pthread_mutex_trylock(&_lock);
  assert(result == 0 || result == EBUSY);
  return (result == 0);
}

/**

 */
INLINE void ReMutexPosixImpl::
release() {
  TAU_PROFILE("void ReMutexPosixImpl::release", " ", TAU_USER);
  int result = pthread_mutex_unlock(&_lock);
  assert(result == 0);
}

/**
 * Returns the underlying Posix lock handle.
 */
INLINE pthread_mutex_t *ReMutexPosixImpl::
get_posix_lock() {
  return &_lock;
}
