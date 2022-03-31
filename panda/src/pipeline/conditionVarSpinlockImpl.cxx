/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file conditionVarSpinlockImpl.cxx
 * @author drose
 * @date 2006-04-11
 */

#include "selectThreadImpl.h"

#ifdef MUTEX_SPINLOCK

#include "conditionVarSpinlockImpl.h"
#include "trueClock.h"

#if defined(__i386__) || defined(__x86_64) || defined(_M_IX86) || defined(_M_X64)
#include <emmintrin.h>
#define PAUSE() _mm_pause()
#else
#define PAUSE()
#endif

/**
 *
 */
void ConditionVarSpinlockImpl::
wait() {
  AtomicAdjust::Integer current = _event;
  _mutex.unlock();

  while (AtomicAdjust::get(_event) == current) {
    PAUSE();
  }

  _mutex.lock();
}

/**
 *
 */
void ConditionVarSpinlockImpl::
wait(double timeout) {
  TrueClock *clock = TrueClock::get_global_ptr();
  double end_time = clock->get_short_time() + timeout;

  AtomicAdjust::Integer current = _event;
  _mutex.unlock();

  while (AtomicAdjust::get(_event) == current && clock->get_short_time() < end_time) {
    PAUSE();
  }

  _mutex.lock();
}

#undef PAUSE

#endif  // MUTEX_SPINLOCK
