/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file conditionVarDummyImpl.h
 * @author drose
 * @date 2002-08-09
 */

#ifndef CONDITIONVARDUMMYIMPL_H
#define CONDITIONVARDUMMYIMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"
#include "thread.h"

#include "pnotify.h"

class MutexDummyImpl;

/**
 * A fake condition variable implementation for single-threaded applications
 * that don't need any synchronization control.
 */
class EXPCL_PANDA_PIPELINE ConditionVarDummyImpl {
public:
  INLINE ConditionVarDummyImpl(MutexDummyImpl &mutex);
  INLINE ~ConditionVarDummyImpl();

  INLINE void wait();
  INLINE void wait(double timeout);
  INLINE void notify();
  INLINE void notify_all();
};

#include "conditionVarDummyImpl.I"

#endif
