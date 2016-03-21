/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file conditionVarFullWin32Impl.h
 * @author drose
 * @date 2006-08-28
 */

#ifndef CONDITIONVARFULLWIN32IMPL_H
#define CONDITIONVARFULLWIN32IMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#if defined(WIN32_VC)

#include "mutexWin32Impl.h"
#include "pnotify.h"
#include "atomicAdjust.h"

class MutexWin32Impl;

/**
 * Uses Windows native calls to implement a conditionVarFull.
 *
 * We follow the "SetEvent" implementation suggested by
 * http://www.cs.wustl.edu/~schmidt/win32-cv-1.html . This allows us to
 * implement both notify() and notify_all(), but it has more overhead than the
 * simpler implementation of ConditionVarWin32Impl.
 *
 * As described by the above reference, this implementation suffers from a few
 * weaknesses; in particular, it does not necessarily wake up all threads
 * fairly; and it may sometimes incorrectly wake up a thread that was not
 * waiting at the time notify() was called.  But we figure it's good enough
 * for our purposes.
 */
class EXPCL_PANDA_PIPELINE ConditionVarFullWin32Impl {
public:
  INLINE ConditionVarFullWin32Impl(MutexWin32Impl &mutex);
  INLINE ~ConditionVarFullWin32Impl();

  INLINE void wait();
  INLINE void wait(double timeout);
  INLINE void notify();
  INLINE void notify_all();

private:
  CRITICAL_SECTION *_external_mutex;
  HANDLE _event_signal;
  HANDLE _event_broadcast;
  TVOLATILE AtomicAdjust::Integer _waiters_count;
};

#include "conditionVarFullWin32Impl.I"

#endif  // WIN32_VC

#endif
