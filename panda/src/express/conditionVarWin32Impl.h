// Filename: conditionVarWin32Impl.h
// Created by:  drose (07Feb06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef CONDITIONVARWIN32IMPL_H
#define CONDITIONVARWIN32IMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#ifdef THREAD_WIN32_IMPL

#include "mutexWin32Impl.h"
#include "pnotify.h"

#include <prcvar.h>

class MutexWin32Impl;

////////////////////////////////////////////////////////////////////
//       Class : ConditionVarWin32Impl
// Description : Uses Windows native calls to implement a
//               conditionVar.
//
//               The Windows native synchronization primitives don't
//               actually implement a full POSIX-style condition
//               variable, but the Event primitive does a fair job if
//               we disallow POSIX broadcast.  See
//               http://www.cs.wustl.edu/~schmidt/win32-cv-1.html for
//               a full implementation that includes broadcast.  This
//               class is much simpler than that full implementation,
//               so we can avoid the overhead require to support
//               broadcast.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS ConditionVarWin32Impl {
public:
  INLINE ConditionVarWin32Impl(MutexWin32Impl &mutex);
  INLINE ~ConditionVarWin32Impl();

  INLINE void wait();
  INLINE void signal();

private:
  CRITICAL_SECTION *_external_mutex;
  HANDLE _event_signal;
};

#include "conditionVarWin32Impl.I"

#endif  // THREAD_WIN32_IMPL

#endif
