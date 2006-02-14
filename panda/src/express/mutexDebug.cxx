// Filename: mutexDebug.cxx
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

#include "mutexDebug.h"
#include "thread.h"

#ifdef DEBUG_THREADS

MutexImpl MutexDebug::_global_mutex;

////////////////////////////////////////////////////////////////////
//     Function: MutexDebug::Destructor
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
MutexDebug::
~MutexDebug() {
  nassertv(_locking_thread == NULL && _lock_count == 0);

  // Put a distinctive, bogus lock count in upon destruction, so we'll
  // be more likely to notice a floating pointer.
  _lock_count = -100;
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDebug::output
//       Access: Public, Virtual
//  Description: This method is declared virtual in MutexDebug, but
//               non-virtual in MutexDirect.
////////////////////////////////////////////////////////////////////
void MutexDebug::
output(ostream &out) const {
  if (_allow_recursion) {
    out << "ReMutex " << (void *)this;
  } else {
    out << "Mutex " << (void *)this;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDebug::do_lock
//       Access: Private
//  Description: The private implementation of lock() assumes that
//               _global_mutex is held.
////////////////////////////////////////////////////////////////////
void MutexDebug::
do_lock() {
  // If this assertion is triggered, you tried to lock a
  // recently-destructed mutex.
  nassertv(_lock_count != -100);

  Thread *this_thread = Thread::get_current_thread();

  if (_locking_thread == (Thread *)NULL) {
    // The mutex is not already locked by anyone.  Lock it.
    _locking_thread = this_thread;
    ++_lock_count;
    nassertv(_lock_count == 1);

  } else if (_locking_thread == this_thread) {
    // The mutex is already locked by this thread.  Increment the lock
    // count.
    nassertv(_lock_count >= 0);
    if (!_allow_recursion && _lock_count == 0) {
      ostringstream ostr;
      ostr << *_locking_thread << " attempted to re-lock non-reentrant "
           << *this;
      nassert_raise(ostr.str());
      return;
    }
    ++_lock_count;

  } else {
    // The mutex is locked by some other thread.  Check for deadlock?
    MutexDebug *next_mutex = this;

    while (next_mutex != NULL) {
      if (next_mutex->_locking_thread == this_thread) {
        // Whoops, the thread is blocked on me!  Deadlock!
        report_deadlock(this_thread);
        nassert_raise("Deadlock");
        _global_mutex.release();
        return;
      }
      Thread *next_thread = next_mutex->_locking_thread;
      if (next_thread == NULL) {
        // Looks like this mutex isn't actually locked, which means
        // the last thread isn't really blocked--it just hasn't woken
        // up yet to discover that.  In any case, no deadlock.
        break;
      }

      // The last thread is blocked on this "next thread"'s mutex, but
      // what mutex is the next thread blocked on?
      next_mutex = next_thread->_blocked_on_mutex;
    }

    // OK, no deadlock detected.  Carry on.
    this_thread->_blocked_on_mutex = this;

    // Go to sleep on the condition variable until it's unlocked.

    if (thread_cat.is_spam()) {
      thread_cat.spam()
        << *this_thread << " blocking on " << *this << "\n";
    }
    while (_locking_thread != (Thread *)NULL) {
      _cvar.wait();
    }
    if (thread_cat.is_spam()) {
      thread_cat.spam()
        << *this_thread << " awake\n";
    }

    this_thread->_blocked_on_mutex = NULL;

    _locking_thread = this_thread;
    ++_lock_count;
    nassertv(_lock_count == 1);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDebug::do_release
//       Access: Private
//  Description: The private implementation of lock() assumes that
//               _global_mutex is held.
////////////////////////////////////////////////////////////////////
void MutexDebug::
do_release() {
  // If this assertion is triggered, you tried to release a
  // recently-destructed mutex.
  nassertv(_lock_count != -100);

  Thread *this_thread = Thread::get_current_thread();

  if (_locking_thread != this_thread) {
    ostringstream ostr;
    ostr << *this_thread << " attempted to release "
         << *this << " which it does not own";
    nassert_raise(ostr.str());
    _global_mutex.release();
    return;
  }

  nassertv(_lock_count > 0);

  --_lock_count;
  if (_lock_count == 0) {
    // That was the last lock held by this thread.  Release the lock.
    _locking_thread = (Thread *)NULL;
    _cvar.signal();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDebug::do_debug_is_locked
//       Access: Private
//  Description: The private implementation of debug_is_locked()
//               assumes that _global_mutex is held.
////////////////////////////////////////////////////////////////////
bool MutexDebug::
do_debug_is_locked() const {
  return (_locking_thread == Thread::get_current_thread());
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDebug::report_deadlock
//       Access: Private
//  Description: Reports a detected deadlock situation.  _global_mutex
//               should be already held.
////////////////////////////////////////////////////////////////////
void MutexDebug::
report_deadlock(Thread *this_thread) {
  thread_cat.error()
    << "\n\n"
    << "****************************************************************\n"
    << "*****                 Deadlock detected!                   *****\n"
    << "****************************************************************\n"
    << "\n";

  thread_cat.error()
    << *this_thread << " attempted to lock " << *this
    << " which is held by " << *_locking_thread << "\n";

  MutexDebug *next_mutex = this;
  Thread *next_thread = next_mutex->_locking_thread;
  next_mutex = next_thread->_blocked_on_mutex;
  while (next_mutex != NULL) {
    thread_cat.error()
      << *next_thread << " is blocked waiting on "
      << *next_mutex << " which is held by "
      << *next_mutex->_locking_thread << "\n";
    next_thread = next_mutex->_locking_thread;
    next_mutex = next_thread->_blocked_on_mutex;
  }

  thread_cat.error() 
    << "Deadlock!\n";
}

#endif  //  DEBUG_THREADS
