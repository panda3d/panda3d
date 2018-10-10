/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file conditionVarSpinlockImpl.h
 * @author drose
 * @date 2006-04-11
 */

#ifndef CONDITIONVARSPINLOCKIMPL_H
#define CONDITIONVARSPINLOCKIMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#ifdef MUTEX_SPINLOCK

#include "mutexSpinlockImpl.h"
#include "pnotify.h"
#include "atomicAdjust.h"

class MutexSpinlockImpl;

/**
 * Uses a simple user-space spinlock to implement a condition variable.  It is
 * usually not a good idea to use this implementation, unless you are building
 * Panda for a specific application on a specific SMP machine, and you are
 * confident that you have at least as many CPU's as you have threads.
 */
class EXPCL_PANDA_PIPELINE ConditionVarSpinlockImpl {
public:
  INLINE ConditionVarSpinlockImpl(MutexSpinlockImpl &mutex);
  INLINE ~ConditionVarSpinlockImpl();

  void wait();
  void wait(double timeout);
  INLINE void notify();
  INLINE void notify_all();

private:
  MutexSpinlockImpl &_mutex;
  TVOLATILE AtomicAdjust::Integer _event;
};

#include "conditionVarSpinlockImpl.I"

#endif  // MUTEX_SPINLOCK

#endif
