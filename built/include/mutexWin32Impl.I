/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mutexWin32Impl.I
 * @author drose
 * @date 2006-02-07
 */

/**
 *
 */
INLINE void MutexWin32Impl::
lock() {
  AcquireSRWLockExclusive(&_lock);
}

/**
 *
 */
INLINE bool MutexWin32Impl::
try_lock() {
  return (TryAcquireSRWLockExclusive(&_lock) != 0);
}

/**
 *
 */
INLINE void MutexWin32Impl::
unlock() {
  ReleaseSRWLockExclusive(&_lock);
}

/**
 *
 */
INLINE ReMutexWin32Impl::
~ReMutexWin32Impl() {
  DeleteCriticalSection(&_lock);
}

/**
 *
 */
INLINE void ReMutexWin32Impl::
lock() {
  EnterCriticalSection(&_lock);
}

/**
 *
 */
INLINE bool ReMutexWin32Impl::
try_lock() {
  return (TryEnterCriticalSection(&_lock) != 0);
}

/**
 *
 */
INLINE void ReMutexWin32Impl::
unlock() {
  LeaveCriticalSection(&_lock);
}
