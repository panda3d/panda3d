/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file reMutexSpinlockImpl.cxx
 * @author rdb
 * @date 2018-09-03
 */

#include "selectThreadImpl.h"

#ifdef MUTEX_SPINLOCK

#include "reMutexSpinlockImpl.h"
#include "thread.h"

#if defined(__i386__) || defined(__x86_64) || defined(_M_IX86) || defined(_M_X64)
#include <emmintrin.h>
#define PAUSE() _mm_pause()
#else
#define PAUSE()
#endif

/**
 *
 */
void ReMutexSpinlockImpl::
lock() {
  Thread *current_thread = Thread::get_current_thread();
  Thread *locking_thread = (Thread *)AtomicAdjust::compare_and_exchange_ptr(_locking_thread, nullptr, current_thread);
  while (locking_thread != nullptr && locking_thread != current_thread) {
    PAUSE();
    locking_thread = (Thread *)AtomicAdjust::compare_and_exchange_ptr(_locking_thread, nullptr, current_thread);
  }
  ++_counter;
}

/**
 *
 */
bool ReMutexSpinlockImpl::
try_lock() {
  Thread *current_thread = Thread::get_current_thread();
  Thread *locking_thread = (Thread *)AtomicAdjust::compare_and_exchange_ptr(_locking_thread, nullptr, current_thread);
  if (locking_thread == nullptr || locking_thread == current_thread) {
    ++_counter;
    return true;
  } else {
    return false;
  }
}

#undef PAUSE

#endif  // MUTEX_SPINLOCK
