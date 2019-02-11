/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file conditionVarWin32Impl.h
 * @author drose
 * @date 2006-02-07
 */

#ifndef CONDITIONVARWIN32IMPL_H
#define CONDITIONVARWIN32IMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#if defined(WIN32_VC)

#include "mutexWin32Impl.h"
#include "pnotify.h"

class MutexWin32Impl;

/**
 * Uses Windows native calls to implement a ConditionVar.
 *
 * On Windows Vista and above, we use the system's native condition variables.
 *
 * On Windows XP, we follow the "SetEvent" implementation suggested by
 * http://www.cs.wustl.edu/~schmidt/win32-cv-1.html . This allows us to
 * implement both notify() and notify_all().
 *
 * As described by the above reference, this implementation suffers from a few
 * weaknesses; in particular, it does not necessarily wake up all threads
 * fairly; and it may sometimes incorrectly wake up a thread that was not
 * waiting at the time notify() was called.  But we figure it's good enough
 * for our purposes.
 */
class EXPCL_PANDA_PIPELINE ConditionVarWin32Impl {
public:
  INLINE ConditionVarWin32Impl(MutexWin32Impl &mutex);
  INLINE ~ConditionVarWin32Impl();

  INLINE void wait();
  INLINE void wait(double timeout);
  INLINE void notify();
  INLINE void notify_all();

private:
  MutexWin32Impl &_mutex;

  // On Windows XP, the first field contains a Signal (auto-reset) event,
  // the second field a broadcast (manual reset) event, third a waiter count.
  // On Windows Vista and above, the first contains a PCONDITION_VARIABLE.
  volatile PVOID _cvar[3] = {nullptr, nullptr, nullptr};
};

#include "conditionVarWin32Impl.I"

#endif  // WIN32_VC

#endif
