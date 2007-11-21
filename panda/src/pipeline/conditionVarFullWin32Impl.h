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
#include "atomicAdjust.h"

class MutexWin32Impl;

////////////////////////////////////////////////////////////////////
//       Class : ConditionVarFullWin32Impl
// Description : Uses Windows native calls to implement a
//               conditionVarFull.
//
//               We follow the "SetEvent" implementation suggested by
//               http://www.cs.wustl.edu/~schmidt/win32-cv-1.html .
//               This allows us to implement both signal() and
//               signal_all(), but it has more overhead than the
//               simpler implementation of ConditionVarWin32Impl.
//
//               As described by the above reference, this
//               implementation suffers from a few weaknesses; in
//               particular, it does not necessarily wake up all
//               threads fairly; and it may sometimes incorrectly wake
//               up a thread that was not waiting at the time signal()
//               was called.  But we figure it's good enough for our
//               purposes.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PIPELINE ConditionVarFullWin32Impl {
public:
  INLINE ConditionVarFullWin32Impl(MutexWin32Impl &mutex);
  INLINE ~ConditionVarFullWin32Impl();

  INLINE void wait();
  INLINE void signal();
  INLINE void signal_all();

private:
  CRITICAL_SECTION *_external_mutex;
  HANDLE _event_signal;
  HANDLE _event_broadcast;
  TVOLATILE AtomicAdjust::Integer _waiters_count;
};

#include "conditionVarFullWin32Impl.I"

#endif  // WIN32_VC

#endif
