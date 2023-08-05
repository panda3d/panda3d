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

#ifdef _WIN32

#include "mutexWin32Impl.h"
#include "pnotify.h"

class MutexWin32Impl;

/**
 * Uses Windows native calls to implement a ConditionVar.
 */
class EXPCL_PANDA_PIPELINE ConditionVarWin32Impl {
public:
  INLINE ConditionVarWin32Impl(MutexWin32Impl &mutex);
  ~ConditionVarWin32Impl() = default;

  INLINE void wait();
  INLINE void wait(double timeout);
  INLINE void notify();
  INLINE void notify_all();

private:
  MutexWin32Impl &_mutex;
  CONDITION_VARIABLE _cvar = CONDITION_VARIABLE_INIT;

  static BOOL (__stdcall *_wait_func)(PCONDITION_VARIABLE, PSRWLOCK, DWORD, ULONG);
  friend class PStatClientImpl;
};

#include "conditionVarWin32Impl.I"

#endif  // _WIN32

#endif
