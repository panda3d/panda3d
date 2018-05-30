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
 *
 */
constexpr MutexPosixImpl::
MutexPosixImpl() noexcept : _lock(PTHREAD_MUTEX_INITIALIZER) {
}

/**
 *
 */
INLINE MutexPosixImpl::
~MutexPosixImpl() {
  TAU_PROFILE("MutexPosixImpl::~MutexPosixImpl", " ", TAU_USER);
  int result = pthread_mutex_destroy(&_lock);
  assert(result == 0);
}

/**
 *
 */
INLINE void MutexPosixImpl::
lock() {
  TAU_PROFILE("void MutexPosixImpl::lock", " ", TAU_USER);
  int result = pthread_mutex_lock(&_lock);
  assert(result == 0);
}

/**
 *
 */
INLINE bool MutexPosixImpl::
try_lock() {
  TAU_PROFILE("bool MutexPosixImpl::try_lock", " ", TAU_USER);
  int result = pthread_mutex_trylock(&_lock);
  assert(result == 0 || result == EBUSY);
  return (result == 0);
}

/**
 *
 */
INLINE void MutexPosixImpl::
unlock() {
  TAU_PROFILE("void MutexPosixImpl::unlock", " ", TAU_USER);
  int result = pthread_mutex_unlock(&_lock);
  assert(result == 0);
}

/**
 *
 */
#ifdef PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
constexpr ReMutexPosixImpl::
ReMutexPosixImpl() noexcept : _lock(PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP) {
}
#else
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
#endif

/**
 *
 */
INLINE ReMutexPosixImpl::
~ReMutexPosixImpl() {
  TAU_PROFILE("ReMutexPosixImpl::~ReMutexPosixImpl", " ", TAU_USER);
  int result = pthread_mutex_destroy(&_lock);
  assert(result == 0);
}

/**
 *
 */
INLINE void ReMutexPosixImpl::
lock() {
  TAU_PROFILE("void ReMutexPosixImpl::lock", " ", TAU_USER);
  int result = pthread_mutex_lock(&_lock);
  assert(result == 0);
}

/**
 *
 */
INLINE bool ReMutexPosixImpl::
try_lock() {
  TAU_PROFILE("bool ReMutexPosixImpl::try_lock", " ", TAU_USER);
  int result = pthread_mutex_trylock(&_lock);
  assert(result == 0 || result == EBUSY);
  return (result == 0);
}

/**
 *
 */
INLINE void ReMutexPosixImpl::
unlock() {
  TAU_PROFILE("void ReMutexPosixImpl::unlock", " ", TAU_USER);
  int result = pthread_mutex_unlock(&_lock);
  assert(result == 0);
}
