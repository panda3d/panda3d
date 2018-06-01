/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file conditionVarPosixImpl.cxx
 * @author drose
 * @date 2006-02-10
 */

#include "selectThreadImpl.h"

#ifdef HAVE_POSIX_THREADS

#include "conditionVarPosixImpl.h"
#include <sys/time.h>

/**
 *
 */
void ConditionVarPosixImpl::
wait(double timeout) {
  // TAU_PROFILE("ConditionVarPosixImpl::wait()", " ", TAU_USER);

  struct timeval now;
  gettimeofday(&now, nullptr);

  // Convert from timeval to timespec
  struct timespec ts;
  ts.tv_sec  = now.tv_sec;
  ts.tv_nsec = now.tv_usec * 1000;

  int seconds = (int)floor(timeout);
  ts.tv_sec += seconds;
  ts.tv_nsec += (int)((timeout - seconds) * 1000000.0);
  if (ts.tv_nsec > 1000000) {
    ts.tv_nsec -= 1000000;
    ++ts.tv_sec;
  }

  int result = pthread_cond_timedwait(&_cvar, &_mutex._lock, &ts);
#ifndef NDEBUG
  if (result != 0 && result != ETIMEDOUT) {
    pipeline_cat.error()
      << "Unexpected error " << result << " from pthread_cond_timedwait()\n";
  }
#endif
}

#endif  // HAVE_POSIX_THREADS
