/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mutexSpinlockImpl.h
 * @author drose
 * @date 2006-04-11
 */

#ifndef MUTEXSPINLOCKIMPL_H
#define MUTEXSPINLOCKIMPL_H

#include "dtoolbase.h"
#include "selectThreadImpl.h"

#ifdef MUTEX_SPINLOCK

#include "atomicAdjust.h"

/**
 * Uses a simple user-space spinlock to implement a mutex.  It is usually not
 * a good idea to use this implementation, unless you are building Panda for a
 * specific application on a specific SMP machine, and you are confident that
 * you have at least as many CPU's as you have threads.
 */
class EXPCL_DTOOL MutexSpinlockImpl {
public:
  CONSTEXPR MutexSpinlockImpl();

private:
  MutexSpinlockImpl(const MutexSpinlockImpl &copy) DELETED;
  MutexSpinlockImpl &operator = (const MutexSpinlockImpl &copy) DELETED_ASSIGN;

public:
  INLINE void acquire();
  INLINE bool try_acquire();
  INLINE void release();

private:
  void do_lock();

  TVOLATILE AtomicAdjust::Integer _lock;
};

#include "mutexSpinlockImpl.I"

#endif  // MUTEX_SPINLOCK

#endif
