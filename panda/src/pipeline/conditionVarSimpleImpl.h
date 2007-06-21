// Filename: conditionVarSimpleImpl.h
// Created by:  drose (19Jun07)
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

#ifndef CONDITIONVARSIMPLEIMPL_H
#define CONDITIONVARSIMPLEIMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#ifdef THREAD_SIMPLE_IMPL

#include "blockerSimple.h"
#include "mutexTrueImpl.h"

////////////////////////////////////////////////////////////////////
//       Class : ConditionVarSimpleImpl
// Description : Implements a simple condition variable using
//               simulated user-space threads.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ConditionVarSimpleImpl : public BlockerSimple {
public:
  INLINE ConditionVarSimpleImpl(MutexTrueImpl &mutex);
  INLINE ~ConditionVarSimpleImpl();

  void wait();
  INLINE void signal();
  INLINE void signal_all();

private:
  void do_signal();
  void do_signal_all();

  MutexTrueImpl &_mutex;
};

#include "conditionVarSimpleImpl.I"

#endif  // THREAD_SIMPLE_IMPL

#endif
