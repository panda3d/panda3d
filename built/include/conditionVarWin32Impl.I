/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file conditionVarWin32Impl.I
 * @author drose
 * @date 2006-02-07
 */

/**
 *
 */
INLINE ConditionVarWin32Impl::
ConditionVarWin32Impl(MutexWin32Impl &mutex) : _mutex(mutex) {
}

/**
 *
 */
INLINE void ConditionVarWin32Impl::
wait() {
#ifdef DO_PSTATS
  _wait_func(&_cvar, &_mutex._lock, INFINITE, 0);
#else
  SleepConditionVariableSRW(&_cvar, &_mutex._lock, INFINITE, 0);
#endif
}

/**
 *
 */
INLINE void ConditionVarWin32Impl::
wait(double timeout) {
#ifdef DO_PSTATS
  _wait_func(&_cvar, &_mutex._lock, (DWORD)(timeout * 1000.0), 0);
#else
  SleepConditionVariableSRW(&_cvar, &_mutex._lock, (DWORD)(timeout * 1000.0), 0);
#endif
}

/**
 *
 */
INLINE void ConditionVarWin32Impl::
notify() {
  WakeConditionVariable(&_cvar);
}

/**
 *
 */
INLINE void ConditionVarWin32Impl::
notify_all() {
  WakeAllConditionVariable(&_cvar);
}
