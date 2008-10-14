// Filename: conditionVarFull.h
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

#ifndef CONDITIONVARFULL_H
#define CONDITIONVARFULL_H

#include "pandabase.h"
#include "conditionVarFullDebug.h"
#include "conditionVarFullDirect.h"

////////////////////////////////////////////////////////////////////
//       Class : ConditionVarFull
// Description : This class implements a condition variable; see
//               ConditionVar for a brief introduction to this class.
//               The ConditionVarFull class provides a more complete
//               implementation than ConditionVar; in particular, it
//               provides the notify_all() method, which is guaranteed
//               to wake up all threads currently waiting on the
//               condition (whereas notify() is guaranteed to wake up
//               at least one thread, but may or may not wake up all
//               of them).
//
//               This class exists because on certain platforms
//               (e.g. Win32), implementing notify_all() requires more
//               overhead, so you should use ConditionVar for cases
//               when you do not require the notify_all() semantics.
//
//               There are still some minor semantics that POSIX
//               condition variables provide which this implementation
//               does not.  For instance, it is required (not
//               optional) that the caller of notify() or notify_all()
//               is holding the condition variable's mutex before the
//               call.
//
//               This class inherits its implementation either from
//               ConditionVarFullDebug or ConditionVarFullDirect,
//               depending on the definition of DEBUG_THREADS.
////////////////////////////////////////////////////////////////////
#ifdef DEBUG_THREADS
class EXPCL_PANDA_PIPELINE ConditionVarFull : public ConditionVarFullDebug
#else
class EXPCL_PANDA_PIPELINE ConditionVarFull : public ConditionVarFullDirect
#endif  // DEBUG_THREADS
{
PUBLISHED:
  INLINE ConditionVarFull(Mutex &mutex);
  INLINE ~ConditionVarFull();
private:
  INLINE ConditionVarFull(const ConditionVarFull &copy);
  INLINE void operator = (const ConditionVarFull &copy);

PUBLISHED:
  INLINE Mutex &get_mutex() const;
};

#include "conditionVarFull.I"

#endif
