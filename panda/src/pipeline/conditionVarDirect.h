// Filename: conditionVarDirect.h
// Created by:  drose (13Feb06)
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

#ifndef CONDITIONVARDIRECT_H
#define CONDITIONVARDIRECT_H

#include "pandabase.h"
#include "mutexDirect.h"
#include "conditionVarImpl.h"

#ifndef DEBUG_THREADS

////////////////////////////////////////////////////////////////////
//       Class : ConditionVarDirect
// Description : A condition variable, usually used to communicate
//               information about changing state to a thread that is
//               waiting for something to happen.  A condition
//               variable can be used to "wake up" a thread when some
//               arbitrary condition has changed.
//
//               A condition variable is associated with a single
//               mutex, and several condition variables may share the
//               same mutex.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PIPELINE ConditionVarDirect {
public:
  INLINE ConditionVarDirect(MutexDirect &mutex);
  INLINE ~ConditionVarDirect();
private:
  INLINE ConditionVarDirect(const ConditionVarDirect &copy);
  INLINE void operator = (const ConditionVarDirect &copy);

PUBLISHED:
  INLINE MutexDirect &get_mutex() const;

  BLOCKING INLINE void wait();
  BLOCKING INLINE void wait(double timeout);
  INLINE void notify();
  void output(ostream &out) const;

private:
  MutexDirect &_mutex;
  ConditionVarImpl _impl;
};

INLINE ostream &
operator << (ostream &out, const ConditionVarDirect &cv) {
  cv.output(out);
  return out;
}

#include "conditionVarDirect.I"

#endif  // !DEBUG_THREADS

#endif
