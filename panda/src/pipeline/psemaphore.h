// Filename: psemaphore.h
// Created by:  drose (13Oct08)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef PSEMAPHORE_H
#define PSEMAPHORE_H

#include "pandabase.h"
#include "pmutex.h"
#include "conditionVar.h"
#include "mutexHolder.h"

////////////////////////////////////////////////////////////////////
//       Class : Semaphore
// Description : A classic semaphore synchronization primitive.  
//
//               A semaphore manages an internal counter which is
//               decremented by each acquire() call and incremented by
//               each release() call. The counter can never go below
//               zero; when acquire() finds that it is zero, it
//               blocks, waiting until some other thread calls
//               release().
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PIPELINE Semaphore {
PUBLISHED:
  INLINE Semaphore(int initial_count = 1);
  INLINE ~Semaphore();
private:
  INLINE Semaphore(const Semaphore &copy);
  INLINE void operator = (const Semaphore &copy);

PUBLISHED:
  BLOCKING INLINE void acquire();
  BLOCKING INLINE bool try_acquire();
  INLINE int release();

  INLINE int get_count() const;
  void output(ostream &out) const;

private:
  Mutex _lock;
  ConditionVar _cvar;
  int _count;
};

INLINE ostream &
operator << (ostream &out, const Semaphore &sem) {
  sem.output(out);
  return out;
}

#include "psemaphore.I"

#endif
