// Filename: conditionVarDummyImpl.h
// Created by:  drose (09Aug02)
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

#ifndef CONDITIONVARDUMMYIMPL_H
#define CONDITIONVARDUMMYIMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#ifdef THREAD_DUMMY_IMPL

#include "notify.h"

class MutexDummyImpl;

////////////////////////////////////////////////////////////////////
//       Class : ConditionVarDummyImpl
// Description : A fake condition variable implementation for
//               single-threaded applications that don't need any
//               synchronization control.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS ConditionVarDummyImpl {
public:
  INLINE ConditionVarDummyImpl(MutexDummyImpl &mutex);
  INLINE ~ConditionVarDummyImpl();

  INLINE void wait();
  INLINE void signal();
  INLINE void signal_all();
};

#include "conditionVarDummyImpl.I"

#endif  // THREAD_DUMMY_IMPL

#endif
