// Filename: reMutex.cxx
// Created by:  drose (15Jan06)
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

#include "reMutex.h"

#ifndef MUTEX_REENTRANT

// Most of the methods in this class are stubbed out in the
// THREAD_DUMMY_IMPL case, especially when CHECK_REENTRANT_MUTEX is
// not defined.  In this case, we're not performing any actual locking
// or verification, so there's no need to do anything at all here.

#if !defined(THREAD_DUMMY_IMPL) || defined(CHECK_REENTRANT_MUTEX)

////////////////////////////////////////////////////////////////////
//     Function: ReMutex::do_lock
//       Access: Private
//  Description: The private implementation of lock().
////////////////////////////////////////////////////////////////////
void ReMutex::
do_lock() {
  MutexHolder holder(_mutex);

  if (_locking_thread == (Thread *)NULL) {
    // The mutex is not already locked by anyone.  Lock it.
    _locking_thread = Thread::get_current_thread();
    ++_lock_count;
    nassertv(_lock_count == 1);

  } else if (_locking_thread == Thread::get_current_thread()) {
    // The mutex is already locked by this thread.  Increment the lock
    // count.
    ++_lock_count;

  } else {
    // The mutex is locked by some other thread.  Go to sleep on the
    // condition variable until it's unlocked.
    while (_locking_thread != (Thread *)NULL) {
      _cvar.wait();
    }
    _locking_thread = Thread::get_current_thread();
    ++_lock_count;
    nassertv(_lock_count == 1);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutex::do_release
//       Access: Private
//  Description: The private implementation of release().
////////////////////////////////////////////////////////////////////
void ReMutex::
do_release() {
  MutexHolder holder(_mutex);

  nassertv(_locking_thread == Thread::get_current_thread());
  nassertv(_lock_count > 0);

  --_lock_count;
  if (_lock_count == 0) {
    // That was the last lock held by this thread.  Release the lock.
    _locking_thread = (Thread *)NULL;
    _cvar.signal();
  }
}

#endif  // !THREAD_DUMMY_IMPL || CHECK_REENTRANT_MUTEX

#endif  // MUTEX_REENTRANT
