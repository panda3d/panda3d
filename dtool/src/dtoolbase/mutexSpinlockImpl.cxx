/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mutexSpinlockImpl.cxx
 * @author drose
 * @date 2006-04-11
 */

#include "selectThreadImpl.h"

#ifdef MUTEX_SPINLOCK

#include "mutexSpinlockImpl.h"

/**
 *
 */
void MutexSpinlockImpl::
do_lock() {
  while (AtomicAdjust::compare_and_exchange(_lock, 0, 1) != 0) {
  }
}

#endif  // MUTEX_SPINLOCK
