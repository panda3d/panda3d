// Filename: conditionVarNsprImpl.h
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

#ifndef CONDITIONVARNSPRIMPL_H
#define CONDITIONVARNSPRIMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#ifdef THREAD_NSPR_IMPL

#include "mutexNsprImpl.h"
#include "notify.h"

#include <prcvar.h>

class MutexNsprImpl;

////////////////////////////////////////////////////////////////////
//       Class : ConditionVarNsprImpl
// Description : Uses NSPR to implement a conditionVar.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS ConditionVarNsprImpl {
public:
  INLINE ConditionVarNsprImpl(MutexNsprImpl &mutex);
  INLINE ~ConditionVarNsprImpl();

  INLINE void wait();
  INLINE void signal();
  INLINE void signal_all();

private:
  PRCondVar *_cvar;
};

#include "conditionVarNsprImpl.I"

#endif  // THREAD_NSPR_IMPL

#endif
