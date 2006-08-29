// Filename: conditionVarFull.h
// Created by:  drose (28Aug06)
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
//               provides the signal_all() method, which is guaranteed
//               to wake up all threads currently waiting on the
//               condition (whereas signal() is guaranteed to wake up
//               at least one thread, but may or may not wake up all
//               of them).
//
//               This class exists because on certain platforms
//               (e.g. Win32), implementing signal_all() requires more
//               overhead, so you should use ConditionVar for cases
//               when you do not require the signal_all() semantics.
//
//               There are still some minor semantics that POSIX
//               condition variables provide which this implementation
//               does not.  For instance, it is required (not
//               optional) that the caller of signal() or signal_all()
//               is holding the condition variable's mutex before the
//               call.
//
//               This class inherits its implementation either from
//               ConditionVarFullDebug or ConditionVarFullDirect,
//               depending on the definition of DEBUG_THREADS.
////////////////////////////////////////////////////////////////////
#ifdef DEBUG_THREADS
class EXPCL_PANDA ConditionVarFull : public ConditionVarFullDebug
#else
class EXPCL_PANDA ConditionVarFull : public ConditionVarFullDirect
#endif  // DEBUG_THREADS
{
public:
  INLINE ConditionVarFull(Mutex &mutex);
  INLINE ~ConditionVarFull();
private:
  INLINE ConditionVarFull(const ConditionVarFull &copy);
  INLINE void operator = (const ConditionVarFull &copy);

public:
  INLINE Mutex &get_mutex() const;
};

#include "conditionVarFull.I"

#endif
