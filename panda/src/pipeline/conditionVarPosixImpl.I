/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file conditionVarPosixImpl.I
 * @author drose
 * @date 2006-02-10
 */

/**
 *
 */
INLINE ConditionVarPosixImpl::
ConditionVarPosixImpl(MutexPosixImpl &mutex) :
  _mutex(mutex)
{
  TAU_PROFILE("ConditionVarPosixImpl::ConditionVarPosixImpl()", " ", TAU_USER);
  int result = pthread_cond_init(&_cvar, nullptr);
  nassertv(result == 0);
}

/**
 *
 */
INLINE ConditionVarPosixImpl::
~ConditionVarPosixImpl() {
  TAU_PROFILE("ConditionVarPosixImpl::~ConditionVarPosixImpl()", " ", TAU_USER);
  int result = pthread_cond_destroy(&_cvar);
  nassertv(result == 0);
}

/**
 *
 */
INLINE void ConditionVarPosixImpl::
wait() {
  TAU_PROFILE("ConditionVarPosixImpl::wait()", " ", TAU_USER);
  int result = pthread_cond_wait(&_cvar, &_mutex._lock);
#ifndef NDEBUG
  if (result != 0) {
    pipeline_cat.error()
      << "Unexpected error " << result << " from pthread_cond_wait()\n";
  }
#endif
}

/**
 *
 */
INLINE void ConditionVarPosixImpl::
notify() {
  TAU_PROFILE("ConditionVarPosixImpl::notify()", " ", TAU_USER);
  int result = pthread_cond_signal(&_cvar);
  nassertv(result == 0);
}

/**
 *
 */
INLINE void ConditionVarPosixImpl::
notify_all() {
  TAU_PROFILE("ConditionVarPosixImpl::notify()", " ", TAU_USER);
  int result = pthread_cond_broadcast(&_cvar);
  nassertv(result == 0);
}
