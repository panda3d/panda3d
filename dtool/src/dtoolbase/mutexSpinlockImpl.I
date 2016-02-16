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

 */
INLINE MutexSpinlockImpl::
MutexSpinlockImpl() {
  _lock = 0;
}

/**

 */
INLINE MutexSpinlockImpl::
~MutexSpinlockImpl() {
}

/**

 */
INLINE void MutexSpinlockImpl::
acquire() {
  if (!try_acquire()) {
    do_lock();
  }
}

/**

 */
INLINE bool MutexSpinlockImpl::
try_acquire() {
  return (AtomicAdjust::compare_and_exchange(_lock, 0, 1) == 0);
}

/**

 */
INLINE void MutexSpinlockImpl::
release() {
  AtomicAdjust::set(_lock, 0);
}
