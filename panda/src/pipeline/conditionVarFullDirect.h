// Filename: conditionVarFullDirect.h
// Created by:  drose (28Aug06)
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

#ifndef CONDITIONVARFULLDIRECT_H
#define CONDITIONVARFULLDIRECT_H

#include "pandabase.h"
#include "mutexDirect.h"
#include "conditionVarImpl.h"

#ifndef DEBUG_THREADS

////////////////////////////////////////////////////////////////////
//       Class : ConditionVarFullDirect
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
class EXPCL_PANDA_PIPELINE ConditionVarFullDirect {
public:
  INLINE ConditionVarFullDirect(MutexDirect &mutex);
  INLINE ~ConditionVarFullDirect();
private:
  INLINE ConditionVarFullDirect(const ConditionVarFullDirect &copy);
  INLINE void operator = (const ConditionVarFullDirect &copy);

PUBLISHED:
  INLINE MutexDirect &get_mutex() const;

  BLOCKING INLINE void wait();
  BLOCKING INLINE void wait(double timeout);
  INLINE void notify();
  INLINE void notify_all();
  void output(ostream &out) const;

private:
  MutexDirect &_mutex;
  ConditionVarFullImpl _impl;
};

INLINE ostream &
operator << (ostream &out, const ConditionVarFullDirect &cv) {
  cv.output(out);
  return out;
}

#include "conditionVarFullDirect.I"

#endif  // !DEBUG_THREADS

#endif
