// Filename: conditionVarPosixImpl.h
// Created by:  drose (10Feb06)
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

#ifndef CONDITIONVARPOSIXIMPL_H
#define CONDITIONVARPOSIXIMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#ifdef THREAD_POSIX_IMPL

#include "mutexPosixImpl.h"
#include "notify.h"

#include <pthread.h>

class MutexPosixImpl;

////////////////////////////////////////////////////////////////////
//       Class : ConditionVarPosixImpl
// Description : Uses Posix threads to implement a conditionVar.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS ConditionVarPosixImpl {
public:
  INLINE ConditionVarPosixImpl(MutexPosixImpl &mutex);
  INLINE ~ConditionVarPosixImpl();

  INLINE void wait();
  INLINE void signal();

private:
  MutexPosixImpl &_mutex;
  pthread_cond_t _cvar;
};

#include "conditionVarPosixImpl.I"

#endif  // THREAD_POSIX_IMPL

#endif
