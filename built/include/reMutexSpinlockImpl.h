/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file reMutexSpinlockImpl.h
 * @author rdb
 * @date 2018-09-03
 */

#ifndef REMUTEXSPINLOCKIMPL_H
#define REMUTEXSPINLOCKIMPL_H

#include "dtoolbase.h"
#include "selectThreadImpl.h"

#ifdef MUTEX_SPINLOCK

#include "atomicAdjust.h"

class Thread;

/**
 * Uses a simple user-space spinlock to implement a mutex.  It is usually not
 * a good idea to use this implementation, unless you are building Panda for a
 * specific application on a specific SMP machine, and you are confident that
 * you have at least as many CPU's as you have threads.
 */
class EXPCL_PANDA_PIPELINE ReMutexSpinlockImpl {
public:
  constexpr ReMutexSpinlockImpl() noexcept = default;
  ReMutexSpinlockImpl(const ReMutexSpinlockImpl &copy) = delete;

  ReMutexSpinlockImpl &operator = (const ReMutexSpinlockImpl &copy) = delete;

public:
  void lock();
  bool try_lock();
  INLINE void unlock();

private:
  AtomicAdjust::Pointer _locking_thread = nullptr;
  unsigned int _counter = 0;
};


#include "reMutexSpinlockImpl.I"

#endif  // MUTEX_SPINLOCK

#endif
