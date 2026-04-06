/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file reMutexSpinlockImpl.I
 * @author rdb
 * @date 2018-09-03
 */

/**
 *
 */
INLINE void ReMutexSpinlockImpl::
unlock() {
  assert(_counter > 0);
  if (!--_counter) {
    AtomicAdjust::set_ptr(_locking_thread, nullptr);
  }
}
