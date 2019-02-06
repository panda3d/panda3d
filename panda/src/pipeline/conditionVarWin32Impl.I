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
  // Create an auto-reset event.
  _event_signal = CreateEvent(nullptr, false, false, nullptr);
}

/**
 *
 */
INLINE ConditionVarWin32Impl::
~ConditionVarWin32Impl() {
  CloseHandle(_event_signal);
}

/**
 *
 */
INLINE void ConditionVarWin32Impl::
wait() {
  _mutex.unlock();

  DWORD result = WaitForSingleObject(_event_signal, INFINITE);
  nassertv(result == WAIT_OBJECT_0);

  _mutex.lock();
}

/**
 *
 */
INLINE void ConditionVarWin32Impl::
wait(double timeout) {
  _mutex.unlock();

  DWORD result = WaitForSingleObject(_event_signal, (DWORD)(timeout * 1000.0));
  nassertv(result == WAIT_OBJECT_0 || result == WAIT_TIMEOUT);

  _mutex.lock();
}

/**
 *
 */
INLINE void ConditionVarWin32Impl::
notify() {
  SetEvent(_event_signal);
}
