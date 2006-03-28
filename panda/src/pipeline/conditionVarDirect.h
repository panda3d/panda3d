// Filename: conditionVarDirect.h
// Created by:  drose (13Feb06)
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
class EXPCL_PANDA ConditionVarDirect {
public:
  INLINE ConditionVarDirect(MutexDirect &mutex);
  INLINE ~ConditionVarDirect();
private:
  INLINE ConditionVarDirect(const ConditionVarDirect &copy);
  INLINE void operator = (const ConditionVarDirect &copy);

public:
  INLINE MutexDirect &get_mutex() const;

  INLINE void wait();
  INLINE void signal();
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
