// Filename: mutexWin32Impl.I
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
//     Function: MutexWin32Impl::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MutexWin32Impl::
~MutexWin32Impl() {
  DeleteCriticalSection(&_lock);
}

////////////////////////////////////////////////////////////////////
//     Function: MutexWin32Impl::lock
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void MutexWin32Impl::
lock() {
  EnterCriticalSection(&_lock);
}

////////////////////////////////////////////////////////////////////
//     Function: MutexWin32Impl::try_lock
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool MutexWin32Impl::
try_lock() {
  return (TryEnterCriticalSection(&_lock) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: MutexWin32Impl::release
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void MutexWin32Impl::
release() {
  LeaveCriticalSection(&_lock);
}
