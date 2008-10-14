// Filename: conditionVarPosixImpl.h
// Created by:  drose (10Feb06)
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

#ifndef CONDITIONVARPOSIXIMPL_H
#define CONDITIONVARPOSIXIMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#ifdef HAVE_POSIX_THREADS

#include "mutexPosixImpl.h"
#include "pnotify.h"
#include "config_pipeline.h"

#include <pthread.h>

class MutexPosixImpl;

////////////////////////////////////////////////////////////////////
//       Class : ConditionVarPosixImpl
// Description : Uses Posix threads to implement a conditionVar.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PIPELINE ConditionVarPosixImpl {
public:
  INLINE ConditionVarPosixImpl(MutexPosixImpl &mutex);
  INLINE ~ConditionVarPosixImpl();

  INLINE void wait();
  void wait(double timeout);
  INLINE void notify();
  INLINE void notify_all();

private:
  MutexPosixImpl &_mutex;
  pthread_cond_t _cvar;
};

#include "conditionVarPosixImpl.I"

#endif  // HAVE_POSIX_THREADS

#endif
