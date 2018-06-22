/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dConditionVar.cxx
 * @author drose
 * @date 2009-07-02
 */

#include "p3dConditionVar.h"

#ifndef _WIN32
#include <sys/time.h>
#include <math.h>
#endif
#include <errno.h>

/**
 *
 */
P3DConditionVar::
P3DConditionVar() {
#ifdef _WIN32
  InitializeCriticalSection(&_lock);

  // Create an auto-reset event.
  _event_signal = CreateEvent(nullptr, false, false, nullptr);

#else  // _WIN32
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
  int result = pthread_mutex_init(&_lock, &attr);
  pthread_mutexattr_destroy(&attr);
  assert(result == 0);

  result = pthread_cond_init(&_cvar, nullptr);
  assert(result == 0);

#endif  // _WIN32
}

/**
 *
 */
P3DConditionVar::
~P3DConditionVar() {
#ifdef _WIN32
  DeleteCriticalSection(&_lock);
  CloseHandle(_event_signal);

#else  // _WIN32
  int result = pthread_mutex_destroy(&_lock);
  assert(result == 0);

  result = pthread_cond_destroy(&_cvar);
  assert(result == 0);

#endif  // _WIN32
}

/**
 * Acquires the internal lock.  The lock should be held during any calls to
 * wait() or notify().
 */
void P3DConditionVar::
acquire() {
#ifdef _WIN32
  EnterCriticalSection(&_lock);

#else  // _WIN32
  int result = pthread_mutex_lock(&_lock);
  assert(result == 0);

#endif  // _WIN32
}

/**
 * Requires the lock to be held on entry.  Releases the lock, waits for
 * another thread to call notify(), then reacquires the lock on exit.
 */
void P3DConditionVar::
wait() {
#ifdef _WIN32
  LeaveCriticalSection(&_lock);

  DWORD result = WaitForSingleObject(_event_signal, INFINITE);
  assert(result == WAIT_OBJECT_0);

  EnterCriticalSection(&_lock);

#else  // _WIN32
  int result = pthread_cond_wait(&_cvar, &_lock);
  assert(result == 0);

#endif  // _WIN32
}

/**
 * As above, but waits no longer than timeout seconds before returning.
 */
void P3DConditionVar::
wait(double timeout) {
#ifdef _WIN32
  LeaveCriticalSection(&_lock);

  DWORD result = WaitForSingleObject(_event_signal, (DWORD)(timeout * 1000.0));
  assert(result == WAIT_OBJECT_0 || result == WAIT_TIMEOUT);

  EnterCriticalSection(&_lock);
#else  // _WIN32

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

  int result = pthread_cond_timedwait(&_cvar, &_lock, &ts);
  if (result != 0 && result != ETIMEDOUT) {
    errno = result;
    perror("pthread_cond_timedwait");
  }

#endif  // _WIN32
}

/**
 * Waits a single thread blocked on wait(), if any.  If no threads are
 * waiting, the event is lost.  The lock should be held during this call.
 */
void P3DConditionVar::
notify() {
#ifdef _WIN32
  SetEvent(_event_signal);

#else  // _WIN32
  int result = pthread_cond_signal(&_cvar);
  assert(result == 0);

#endif  // _WIN32
}

/**
 * Releases the internal lock.
 */
void P3DConditionVar::
release() {
#ifdef _WIN32
  LeaveCriticalSection(&_lock);

#else  // _WIN32
  int result = pthread_mutex_unlock(&_lock);
  assert(result == 0);

#endif  // _WIN32
}
