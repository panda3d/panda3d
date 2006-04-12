// Filename: conditionVarSpinlockImpl.h
// Created by:  drose (11Apr06)
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

#ifndef CONDITIONVARSPINLOCKIMPL_H
#define CONDITIONVARSPINLOCKIMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#ifdef MUTEX_SPINLOCK

#include "mutexSpinlockImpl.h"
#include "pnotify.h"
#include "atomicAdjust.h"

class MutexSpinlockImpl;

////////////////////////////////////////////////////////////////////
//       Class : ConditionVarSpinlockImpl
// Description : Uses Windows native calls to implement a
//               conditionVar.
//
//               The Windows native synchronization primitives don't
//               actually implement a full POSIX-style condition
//               variable, but the Event primitive does a fair job if
//               we disallow POSIX broadcast.  See
//               http://www.cs.wustl.edu/~schmidt/spinlock-cv-1.html for
//               a full implementation that includes broadcast.  This
//               class is much simpler than that full implementation,
//               so we can avoid the overhead require to support
//               broadcast.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ConditionVarSpinlockImpl {
public:
  INLINE ConditionVarSpinlockImpl(MutexSpinlockImpl &mutex);
  INLINE ~ConditionVarSpinlockImpl();

  void wait();
  INLINE void signal();

private:
  MutexSpinlockImpl &_mutex;
  volatile PN_int32 _event;
};

#include "conditionVarSpinlockImpl.I"

#endif  // MUTEX_SPINLOCK

#endif
