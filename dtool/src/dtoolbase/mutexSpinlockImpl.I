/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mutexSpinlockImpl.I
 * @author drose
 * @date 2006-04-11
 */

/**
 *
 */
INLINE void MutexSpinlockImpl::
lock() {
  if (!try_lock()) {
    do_lock();
  }
}

/**
 *
 */
INLINE bool MutexSpinlockImpl::
try_lock() {
  return !_flag.test_and_set(std::memory_order_acquire);
}

/**
 *
 */
INLINE void MutexSpinlockImpl::
unlock() {
  _flag.clear(std::memory_order_release);
}
