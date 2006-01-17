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

#ifdef CHECK_REENTRANT_MUTEX
////////////////////////////////////////////////////////////////////
//     Function: Mutex::debug_is_locked
//       Access: Public
//  Description: Returns true if the current thread has locked the
//               Mutex, false otherwise.  This method is only
//               meaningful if CHECK_REENTRANT_MUTEX is defined;
//               otherwise, it always returns true.
////////////////////////////////////////////////////////////////////
bool Mutex::
debug_is_locked() const {
  return (_locking_thread == Thread::get_current_thread());
}
#endif  // CHECK_REENTRANT_MUTEX

////////////////////////////////////////////////////////////////////
//     Function: Mutex::do_lock
//       Access: Private
//  Description: The private implementation of lock().
////////////////////////////////////////////////////////////////////
void Mutex::
do_lock() {
#ifdef CHECK_REENTRANT_MUTEX
  nassertv(_locking_thread != Thread::get_current_thread());
#endif
  _impl.lock();

#ifdef CHECK_REENTRANT_MUTEX
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
#ifdef CHECK_REENTRANT_MUTEX
  nassertv(_locking_thread == Thread::get_current_thread());
  _locking_thread = (Thread *)NULL;
#endif

  _impl.release();
}
