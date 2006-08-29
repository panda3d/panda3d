// Filename: conditionVarFullWin32Impl.I
// Created by:  drose (28Aug06)
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


////////////////////////////////////////////////////////////////////
//     Function: ConditionVarFullWin32Impl::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ConditionVarFullWin32Impl::
ConditionVarFullWin32Impl(MutexWin32Impl &mutex) {
  _external_mutex = &mutex._lock;

  // Create an auto-reset event.
  _event_signal = CreateEvent(NULL, false, false, NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarFullWin32Impl::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ConditionVarFullWin32Impl::
~ConditionVarFullWin32Impl() {
  CloseHandle(_event_signal);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarFullWin32Impl::wait
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarFullWin32Impl::
wait() {
  LeaveCriticalSection(_external_mutex);

  DWORD result = WaitForSingleObject(_event_signal, INFINITE);
  nassertv(result == WAIT_OBJECT_0);

  EnterCriticalSection(_external_mutex);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarFullWin32Impl::signal
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarFullWin32Impl::
signal() {
  SetEvent(_event_signal);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarFullWin32Impl::signal_all
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarFullWin32Impl::
signal_all() {
  // TODO.
  nassertv(false);
}
