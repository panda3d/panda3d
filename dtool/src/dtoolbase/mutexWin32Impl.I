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
INLINE MutexWin32Impl::
~MutexWin32Impl() {
  DeleteCriticalSection(&_lock);
}

/**
 *
 */
INLINE void MutexWin32Impl::
lock() {
  EnterCriticalSection(&_lock);
}

/**
 *
 */
INLINE bool MutexWin32Impl::
try_lock() {
  return (TryEnterCriticalSection(&_lock) != 0);
}

/**
 *
 */
INLINE void MutexWin32Impl::
unlock() {
  LeaveCriticalSection(&_lock);
}
