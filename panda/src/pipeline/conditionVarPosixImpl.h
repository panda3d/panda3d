/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file conditionVarPosixImpl.h
 * @author drose
 * @date 2006-02-10
 */

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

/**
 * Uses Posix threads to implement a conditionVar.
 */
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

  static int (*_wait_func)(pthread_cond_t *, pthread_mutex_t *);
  static int (*_timedwait_func)(pthread_cond_t *, pthread_mutex_t *,
                                const struct timespec *);
  friend class PStatClientImpl;
};

#include "conditionVarPosixImpl.I"

#endif  // HAVE_POSIX_THREADS

#endif
