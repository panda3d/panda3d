// Filename: reMutexDirect.cxx
// Created by:  drose (13Feb06)
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

#include "reMutexDirect.h"
#include "thread.h"

#ifndef DEBUG_THREADS

////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::output
//       Access: Published
//  Description: This method is declared virtual in MutexDebug, but
//               non-virtual in ReMutexDirect.
////////////////////////////////////////////////////////////////////
void ReMutexDirect::
output(ostream &out) const {
  out << "ReMutex " << (void *)this;
}

#ifndef HAVE_REMUTEXTRUEIMPL
////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::do_acquire
//       Access: Private
//  Description: The private implementation of acquire(), for the case in
//               which the underlying lock system does not provide a
//               reentrant mutex (and therefore we have to build this
//               functionality on top of the existing non-reentrant
//               mutex).
////////////////////////////////////////////////////////////////////
void ReMutexDirect::
do_acquire(Thread *current_thread) {
  _lock_impl.acquire();

  if (_locking_thread == (Thread *)NULL) {
    // The mutex is not already locked by anyone.  Lock it.
    _locking_thread = current_thread;
    ++_lock_count;
    nassertd(_lock_count == 1) {
    }

  } else if (_locking_thread == current_thread) {
    // The mutex is already locked by this thread.  Increment the lock
    // count.
    ++_lock_count;
    nassertd(_lock_count > 0) {
    }
    
  } else {
    // The mutex is locked by some other thread.  Go to sleep on the
    // condition variable until it's unlocked.
    while (_locking_thread != (Thread *)NULL) {
      _cvar_impl.wait();
    }
    
    _locking_thread = current_thread;
    ++_lock_count;
    nassertd(_lock_count == 1) {
    }
  }
  _lock_impl.release();
}
#endif  // !HAVE_REMUTEXTRUEIMPL

#ifndef HAVE_REMUTEXTRUEIMPL
////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::do_try_acquire
//       Access: Private
//  Description: The private implementation of acquire(false), for the
//               case in which the underlying lock system does not
//               provide a reentrant mutex (and therefore we have to
//               build this functionality on top of the existing
//               non-reentrant mutex).
////////////////////////////////////////////////////////////////////
bool ReMutexDirect::
do_try_acquire(Thread *current_thread) {
  bool acquired = true;
  _lock_impl.acquire();

  if (_locking_thread == (Thread *)NULL) {
    // The mutex is not already locked by anyone.  Lock it.
    _locking_thread = current_thread;
    ++_lock_count;
    nassertd(_lock_count == 1) {
    }

  } else if (_locking_thread == current_thread) {
    // The mutex is already locked by this thread.  Increment the lock
    // count.
    ++_lock_count;
    nassertd(_lock_count > 0) {
    }
    
  } else {
    // The mutex is locked by some other thread.  Return false.
    acquired = false;
  }
  _lock_impl.release();

  return acquired;
}
#endif  // !HAVE_REMUTEXTRUEIMPL

#ifndef HAVE_REMUTEXTRUEIMPL
////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::do_elevate_lock
//       Access: Private
//  Description: The private implementation of acquire(), for the case in
//               which the underlying lock system does not provide a
//               reentrant mutex (and therefore we have to build this
//               functionality on top of the existing non-reentrant
//               mutex).
////////////////////////////////////////////////////////////////////
void ReMutexDirect::
do_elevate_lock() {
  _lock_impl.acquire();

#ifdef _DEBUG
  nassertd(_locking_thread == Thread::get_current_thread()) {
    _lock_impl.release();
    return;
  }
#elif !defined(NDEBUG)
  nassertd(_locking_thread != (Thread *)NULL) {
    _lock_impl.release();
    return;
  }
#endif  // NDEBUG

  // We know the mutex is already locked by this thread.  Increment
  // the lock count.
  ++_lock_count;
  nassertd(_lock_count > 0) {
  }

  _lock_impl.release();
}
#endif  // !HAVE_REMUTEXTRUEIMPL

#ifndef HAVE_REMUTEXTRUEIMPL
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
  _lock_impl.acquire();

#ifdef _DEBUG
  if (_locking_thread != Thread::get_current_thread()) {
    ostringstream ostr;
    ostr << *_locking_thread << " attempted to release "
         << *this << " which it does not own";
    nassert_raise(ostr.str());
    _lock_impl.release();
    return;
  }
#endif  // _DEBUG

  nassertd(_lock_count > 0) {
  }

  --_lock_count;
  if (_lock_count == 0) {
    // That was the last lock held by this thread.  Release the lock.
    _locking_thread = (Thread *)NULL;
    _cvar_impl.notify();
  }
  _lock_impl.release();
}
#endif  // !HAVE_REMUTEXTRUEIMPL

#endif  // !DEBUG_THREADS
