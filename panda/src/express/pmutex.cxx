// Filename: pmutex.cxx
// Created by:  drose (08Aug02)
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

#include "pmutex.h"
#include "thread.h"

#ifndef NDEBUG
////////////////////////////////////////////////////////////////////
//     Function: Mutex::debug_is_locked
//       Access: Public
//  Description: Returns true if the current thread has locked the
//               Mutex, false otherwise.  This method only exists in
//               !NDEBUG mode, so it's only appropriate to call it
//               from within an assert().
////////////////////////////////////////////////////////////////////
bool Mutex::
debug_is_locked() const {
  return (_locking_thread == Thread::get_current_thread());
}
#endif  // NDEBUG

////////////////////////////////////////////////////////////////////
//     Function: Mutex::do_lock
//       Access: Private
//  Description: The private implementation of lock().
////////////////////////////////////////////////////////////////////
void Mutex::
do_lock() {
  nassertv(_locking_thread != Thread::get_current_thread());
  _impl.lock();

#ifndef NDEBUG
  _locking_thread = Thread::get_current_thread();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: Mutex::do_release
//       Access: Private
//  Description: The private implementation of release().
////////////////////////////////////////////////////////////////////
void Mutex::
do_release() {
  nassertv(_locking_thread == Thread::get_current_thread());
#ifndef NDEBUG
  _locking_thread = (Thread *)NULL;
#endif

  _impl.release();
}
