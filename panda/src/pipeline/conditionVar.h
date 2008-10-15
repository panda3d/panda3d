// Filename: conditionVar.h
// Created by:  drose (09Aug02)
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

#ifndef CONDITIONVAR_H
#define CONDITIONVAR_H

#include "pandabase.h"
#include "conditionVarDebug.h"
#include "conditionVarDirect.h"

////////////////////////////////////////////////////////////////////
//       Class : ConditionVar
// Description : A condition variable, usually used to communicate
//               information about changing state to a thread that is
//               waiting for something to happen.  A condition
//               variable can be used to "wake up" a thread when some
//               arbitrary condition has changed.
//
//               The ConditionVar class does not support the full
//               semantics of POSIX condition variables.  In
//               particular, it does not support the broadcast or
//               notify_all function.  See ConditionVarFull for a more
//               complete (but possibly more expensive) API.
//
//               A condition variable is associated with a single
//               mutex, and several condition variables may share the
//               same mutex.
//
//               This class inherits its implementation either from
//               ConditionVarDebug or ConditionVarDirect, depending on
//               the definition of DEBUG_THREADS.
////////////////////////////////////////////////////////////////////
#ifdef DEBUG_THREADS
class EXPCL_PANDA_PIPELINE ConditionVar : public ConditionVarDebug
#else
class EXPCL_PANDA_PIPELINE ConditionVar : public ConditionVarDirect
#endif  // DEBUG_THREADS
{
PUBLISHED:
  INLINE ConditionVar(Mutex &mutex);
  INLINE ~ConditionVar();
private:
  INLINE ConditionVar(const ConditionVar &copy);
  INLINE void operator = (const ConditionVar &copy);

  // These methods are inherited from the base class.
  // INLINE void wait();
  // INLINE void notify();

private:
  // The notify_all() method is specifically *not* provided by
  // ConditionVar.  Use ConditionVarFull if you need to call this
  // method.
  INLINE void notify_all();

PUBLISHED:
  INLINE Mutex &get_mutex() const;
};

#include "conditionVar.I"

#endif
