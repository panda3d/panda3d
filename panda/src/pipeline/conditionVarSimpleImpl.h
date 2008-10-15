// Filename: conditionVarSimpleImpl.h
// Created by:  drose (19Jun07)
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
class EXPCL_PANDA_PIPELINE ConditionVarSimpleImpl : public BlockerSimple {
public:
  INLINE ConditionVarSimpleImpl(MutexTrueImpl &mutex);
  INLINE ~ConditionVarSimpleImpl();

  void wait();
  void wait(double timeout);
  INLINE void notify();
  INLINE void notify_all();

private:
  void do_notify();
  void do_notify_all();

  MutexTrueImpl &_mutex;
};

#include "conditionVarSimpleImpl.I"

#endif  // THREAD_SIMPLE_IMPL

#endif
