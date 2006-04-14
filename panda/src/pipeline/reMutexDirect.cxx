// Filename: reMutexDirect.cxx
// Created by:  drose (13Feb06)
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

#include "reMutexDirect.h"
#include "thread.h"

#ifndef HAVE_REMUTEXIMPL
MutexImpl ReMutexDirect::_global_lock;
#endif  // !HAVE_REMUTEXIMPL

////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::output
//       Access: Public
//  Description: This method is declared virtual in MutexDebug, but
//               non-virtual in ReMutexDirect.
////////////////////////////////////////////////////////////////////
void ReMutexDirect::
output(ostream &out) const {
  out << "ReMutex " << (void *)this;
}

#ifndef HAVE_REMUTEXIMPL
////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::do_lock
//       Access: Private
//  Description: The private implementation of lock(), for the case in
//               which the underlying lock system does not provide a
//               reentrant mutex (and therefore we have to build this
//               functionality on top of the existing non-reentrant
//               mutex).
////////////////////////////////////////////////////////////////////
void ReMutexDirect::
do_lock() {
  _global_lock.lock();

  if (_locking_thread == (Thread *)NULL) {
    // The mutex is not already locked by anyone.  Lock it.
    _locking_thread = Thread::get_current_thread();
    ++_lock_count;
    nassertd(_lock_count == 1) {
    }

  } else if (_locking_thread == Thread::get_current_thread()) {
    // The mutex is already locked by this thread.  Increment the lock
    // count.
    ++_lock_count;
    nassertd(_lock_count > 0) {
    }

  } else {
    // The mutex is locked by some other thread.  Go to sleep on the
    // condition variable until it's unlocked.
    while (_locking_thread != (Thread *)NULL) {
      _cvar.wait();
    }
    
    _locking_thread = Thread::get_current_thread();
    ++_lock_count;
    nassertd(_lock_count == 1) {
    }
  }
  _global_lock.release();
}
#endif  // !HAVE_REMUTEXIMPL

#ifndef HAVE_REMUTEXIMPL
////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::do_release
//       Access: Private
//  Description: The private implementation of release(), for the case
//               in which the underlying lock system does not provide
//               a reentrant mutex (and therefore we have to build
//               this functionality on top of the existing
//               non-reentrant mutex).
////////////////////////////////////////////////////////////////////
void ReMutexDirect::
do_release() {
  _global_lock.lock();

  if (_locking_thread != Thread::get_current_thread()) {
    ostringstream ostr;
    ostr << *_locking_thread << " attempted to release "
         << *this << " which it does not own";
    nassert_raise(ostr.str());
    _global_lock.release();
    return;
  }

  nassertd(_lock_count > 0) {
  }

  --_lock_count;
  if (_lock_count == 0) {
    // That was the last lock held by this thread.  Release the lock.
    _locking_thread = (Thread *)NULL;
    _cvar.signal();
  }
  _global_lock.release();
}
#endif  // !HAVE_REMUTEXIMPL
