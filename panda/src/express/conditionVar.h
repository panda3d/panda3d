// Filename: conditionVar.h
// Created by:  drose (09Aug02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef CONDITIONVAR_H
#define CONDITIONVAR_H

#include "pandabase.h"
#include "pmutex.h"
#include "conditionVarImpl.h"
#include "notify.h"

////////////////////////////////////////////////////////////////////
//       Class : ConditionVar
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
class EXPCL_PANDAEXPRESS ConditionVar {
public:
  INLINE ConditionVar(Mutex &mutex);
  INLINE ~ConditionVar();
private:
  INLINE ConditionVar(const ConditionVar &copy);
  INLINE void operator = (const ConditionVar &copy);

public:
  INLINE Mutex &get_mutex();

  INLINE void wait();
  INLINE void signal();
  INLINE void signal_all();

private:
  Mutex &_mutex;
  ConditionVarImpl _impl;
};

#include "conditionVar.I"

#endif
