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

#if defined(__i386__) || defined(__x86_64) || defined(_M_IX86) || defined(_M_X64)
#include <emmintrin.h>
#define PAUSE() _mm_pause()
#else
#define PAUSE()
#endif

/**
 *
 */
void MutexSpinlockImpl::
do_lock() {
  // Loop until we changed the flag from 0 to 1 (and it wasn't already 1).
  while (_flag.test_and_set(std::memory_order_acquire)) {
    PAUSE();
  }
}

#endif  // MUTEX_SPINLOCK
