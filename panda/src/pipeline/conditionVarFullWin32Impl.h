// Filename: conditionVarFullWin32Impl.h
// Created by:  drose (28Aug06)
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

#ifndef CONDITIONVARFULLWIN32IMPL_H
#define CONDITIONVARFULLWIN32IMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#ifdef WIN32_VC

#include "mutexWin32Impl.h"
#include "pnotify.h"

class MutexWin32Impl;

////////////////////////////////////////////////////////////////////
//       Class : ConditionVarFullWin32Impl
// Description : Uses Windows native calls to implement a
//               conditionVarFull.
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
class EXPCL_PANDA ConditionVarFullWin32Impl {
public:
  INLINE ConditionVarFullWin32Impl(MutexWin32Impl &mutex);
  INLINE ~ConditionVarFullWin32Impl();

  INLINE void wait();
  INLINE void signal();
  INLINE void signal_all();

private:
  CRITICAL_SECTION *_external_mutex;
  HANDLE _event_signal;
};

#include "conditionVarFullWin32Impl.I"

#endif  // WIN32_VC

#endif
