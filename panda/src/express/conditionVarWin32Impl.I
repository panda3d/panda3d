// Filename: conditionVarWin32Impl.I
// Created by:  drose (07Feb06)
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
//     Function: ConditionVarWin32Impl::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ConditionVarWin32Impl::
ConditionVarWin32Impl(MutexWin32Impl &mutex) {
  _external_mutex = &mutex._lock;

  // Create an auto-reset event.
  _event_signal = CreateEvent(NULL, false, false, NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarWin32Impl::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ConditionVarWin32Impl::
~ConditionVarWin32Impl() {
  CloseHandle(_event_signal);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarWin32Impl::wait
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarWin32Impl::
wait() {
  LeaveCriticalSection(_external_mutex);

  DWORD result = WaitForSingleObject(_event_signal, INFINITE);
  nassertv(result == WAIT_OBJECT_0);

  EnterCriticalSection(_external_mutex);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarWin32Impl::signal
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarWin32Impl::
signal() {
  SetEvent(_event_signal);
}
