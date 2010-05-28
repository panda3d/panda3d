// Filename: conditionVarWin32Impl.h
// Created by:  drose (07Feb06)
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

#ifndef CONDITIONVARWIN32IMPL_H
#define CONDITIONVARWIN32IMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#if defined(WIN32_VC)

#include "mutexWin32Impl.h"
#include "pnotify.h"

class MutexWin32Impl;

////////////////////////////////////////////////////////////////////
//       Class : ConditionVarWin32Impl
// Description : Uses Windows native calls to implement a
//               conditionVar.
//
//               The Windows native synchronization primitives don't
//               actually implement a full POSIX-style condition
//               variable, but the Event primitive does a fair job if
//               we disallow notify_all() (POSIX broadcast).  See
//               ConditionVarFullWin32Impl for a full implementation
//               that includes notify_all().  This class is much
//               simpler than that full implementation, so we can
//               avoid the overhead required to support broadcast.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PIPELINE ConditionVarWin32Impl {
public:
  INLINE ConditionVarWin32Impl(MutexWin32Impl &mutex);
  INLINE ~ConditionVarWin32Impl();

  INLINE void wait();
  INLINE void wait(double timeout);
  INLINE void notify();

private:
  CRITICAL_SECTION *_external_mutex;
  HANDLE _event_signal;
};

#include "conditionVarWin32Impl.I"

#endif  // WIN32_VC

#endif
