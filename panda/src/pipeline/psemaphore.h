/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file psemaphore.h
 * @author drose
 * @date 2008-10-13
 */

#ifndef PSEMAPHORE_H
#define PSEMAPHORE_H

#include "pandabase.h"
#include "pmutex.h"
#include "conditionVar.h"
#include "mutexHolder.h"

/**
 * A classic semaphore synchronization primitive.
 *
 * A semaphore manages an internal counter which is decremented by each
 * acquire() call and incremented by each release() call.  The counter can
 * never go below zero; when acquire() finds that it is zero, it blocks,
 * waiting until some other thread calls release().
 */
class EXPCL_PANDA_PIPELINE Semaphore {
PUBLISHED:
  INLINE explicit Semaphore(int initial_count = 1);
  Semaphore(const Semaphore &copy) = delete;
  ~Semaphore() = default;

  Semaphore &operator = (const Semaphore &copy) = delete;

PUBLISHED:
  BLOCKING INLINE void acquire();
  BLOCKING INLINE bool try_acquire();
  INLINE int release();

  INLINE int get_count() const;
  void output(std::ostream &out) const;

private:
  Mutex _lock;
  ConditionVar _cvar;
  int _count;
};

INLINE std::ostream &
operator << (std::ostream &out, const Semaphore &sem) {
  sem.output(out);
  return out;
}

#include "psemaphore.I"

#endif
