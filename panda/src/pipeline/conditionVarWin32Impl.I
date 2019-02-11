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
INLINE ConditionVarWin32Impl::
~ConditionVarWin32Impl() {
  // These fields are only set in the Windows XP implementation, when these
  // both contain events.
  if (_cvar[1] != nullptr) {
    CloseHandle(_cvar[1]);
  }
  if (_cvar[2] != nullptr) {
    CloseHandle(_cvar[2]);
  }
}

/**
 *
 */
INLINE void ConditionVarWin32Impl::
wait() {
  MutexWin32Impl::_funcs._cvar_wait(_cvar, _mutex._lock, INFINITE, 0);
}

/**
 *
 */
INLINE void ConditionVarWin32Impl::
wait(double timeout) {
  MutexWin32Impl::_funcs._cvar_wait(_cvar, _mutex._lock, (DWORD)(timeout * 1000.0), 0);
}

/**
 *
 */
INLINE void ConditionVarWin32Impl::
notify() {
  MutexWin32Impl::_funcs._cvar_notify_one(_cvar);
}

/**
 *
 */
INLINE void ConditionVarWin32Impl::
notify_all() {
  MutexWin32Impl::_funcs._cvar_notify_all(_cvar);
}
