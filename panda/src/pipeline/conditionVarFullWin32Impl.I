// Filename: conditionVarFullWin32Impl.I
// Created by:  drose (28Aug06)
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


////////////////////////////////////////////////////////////////////
//     Function: ConditionVarFullWin32Impl::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ConditionVarFullWin32Impl::
ConditionVarFullWin32Impl(MutexWin32Impl &mutex) {
  _external_mutex = &mutex._lock;

  // Create an auto-reset event and a manual-reset event.
  _event_signal = CreateEvent(NULL, false, false, NULL);
  _event_broadcast = CreateEvent(NULL, true, false, NULL);

  _waiters_count = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarFullWin32Impl::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ConditionVarFullWin32Impl::
~ConditionVarFullWin32Impl() {
  CloseHandle(_event_signal);
  CloseHandle(_event_broadcast);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarFullWin32Impl::wait
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarFullWin32Impl::
wait() {
  AtomicAdjust::inc(_waiters_count);

  // It's ok to release the external_mutex here since Win32
  // manual-reset events maintain state when used with SetEvent().
  // This avoids the "lost wakeup" bug...
  LeaveCriticalSection(_external_mutex);

  // Wait for either event to become signaled due to notify() being
  // called or notify_all() being called.
  int result = WaitForMultipleObjects(2, &_event_signal, FALSE, INFINITE);

  bool nonzero = AtomicAdjust::dec(_waiters_count);
  bool last_waiter = (result == WAIT_OBJECT_0 + 1 && !nonzero);

  // Some thread called notify_all().
  if (last_waiter) {
    // We're the last waiter to be notified or to stop waiting, so
    // reset the manual event. 
    ResetEvent(_event_broadcast); 
  }

  // Reacquire the <external_mutex>.
  EnterCriticalSection(_external_mutex);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarFullWin32Impl::wait
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarFullWin32Impl::
wait(double timeout) {
  AtomicAdjust::inc(_waiters_count);

  // It's ok to release the external_mutex here since Win32
  // manual-reset events maintain state when used with SetEvent().
  // This avoids the "lost wakeup" bug...
  LeaveCriticalSection(_external_mutex);

  // Wait for either event to become signaled due to notify() being
  // called or notify_all() being called.
  int result = WaitForMultipleObjects(2, &_event_signal, FALSE, (DWORD)(timeout * 1000.0));

  bool nonzero = AtomicAdjust::dec(_waiters_count);
  bool last_waiter = (result == WAIT_OBJECT_0 + 1 && !nonzero);

  // Some thread called notify_all().
  if (last_waiter) {
    // We're the last waiter to be notified or to stop waiting, so
    // reset the manual event. 
    ResetEvent(_event_broadcast); 
  }

  // Reacquire the <external_mutex>.
  EnterCriticalSection(_external_mutex);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarFullWin32Impl::notify
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarFullWin32Impl::
notify() {
  bool have_waiters = AtomicAdjust::get(_waiters_count) > 0;

  if (have_waiters) {
    SetEvent(_event_signal);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarFullWin32Impl::notify_all
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarFullWin32Impl::
notify_all() {
  bool have_waiters = AtomicAdjust::get(_waiters_count) > 0;

  if (have_waiters) {
    SetEvent(_event_broadcast);
  }
}
