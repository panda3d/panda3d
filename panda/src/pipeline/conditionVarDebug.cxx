// Filename: conditionVarDebug.cxx
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

#include "conditionVarDebug.h"
#include "thread.h"
#include "config_pipeline.h"

#ifdef DEBUG_THREADS

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarDebug::Constructor
//       Access: Public
//  Description: You must pass in a Mutex to the condition variable
//               constructor.  This mutex may be shared by other
//               condition variables, if desired.  It is the caller's
//               responsibility to ensure the Mutex object does not
//               destruct during the lifetime of the condition
//               variable.
////////////////////////////////////////////////////////////////////
ConditionVarDebug::
ConditionVarDebug(MutexDebug &mutex) :
  _mutex(mutex),
  _impl(*mutex.get_global_lock())
{
  nassertv(!_mutex._allow_recursion);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarDebug::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
ConditionVarDebug::
~ConditionVarDebug() {
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarDebug::wait
//       Access: Published
//  Description: Waits on the condition.  The caller must already be
//               holding the lock associated with the condition
//               variable before calling this function.
//
//               wait() will release the lock, then go to sleep until
//               some other thread calls notify() on this condition
//               variable.  At that time at least one thread waiting
//               on the same ConditionVarDebug will grab the lock again,
//               and then return from wait().
//
//               It is possible that wait() will return even if no one
//               has called notify().  It is the responsibility of the
//               calling process to verify the condition on return
//               from wait, and possibly loop back to wait again if
//               necessary.
//
//               Note the semantics of a condition variable: the mutex
//               must be held before wait() is called, and it will
//               still be held when wait() returns.  However, it will
//               be temporarily released during the wait() call
//               itself.
////////////////////////////////////////////////////////////////////
void ConditionVarDebug::
wait() {
  _mutex._global_lock->acquire();

  Thread *current_thread = Thread::get_current_thread();

  if (!_mutex.do_debug_is_locked()) {
    ostringstream ostr;
    ostr << *current_thread << " attempted to wait on "
         << *this << " without holding " << _mutex;
    nassert_raise(ostr.str());
    _mutex._global_lock->release();
    return;
  }

  if (thread_cat->is_spam()) {
    thread_cat.spam()
      << *current_thread << " waiting on " << *this << "\n";
  }

  nassertd(current_thread->_waiting_on_cvar == NULL &&
           current_thread->_waiting_on_cvar_full == NULL) {
  }
  current_thread->_waiting_on_cvar = this;
  
  _mutex.do_release();
  _impl.wait();  // temporarily releases _global_lock
  _mutex.do_acquire(current_thread);

  nassertd(current_thread->_waiting_on_cvar == this) {
  }
  current_thread->_waiting_on_cvar = NULL;

  if (thread_cat.is_spam()) {
    thread_cat.spam()
      << *current_thread << " awake on " << *this << "\n";
  }

  _mutex._global_lock->release();
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarDebug::wait
//       Access: Published
//  Description: Waits on the condition, with a timeout.  The function
//               will return when the condition variable is notified,
//               or the timeout occurs.  There is no way to directly
//               tell which happened, and it is possible that neither
//               in fact happened (spurious wakeups are possible).
//
//               See wait() with no parameters for more.
////////////////////////////////////////////////////////////////////
void ConditionVarDebug::
wait(double timeout) {
  _mutex._global_lock->acquire();

  Thread *current_thread = Thread::get_current_thread();

  if (!_mutex.do_debug_is_locked()) {
    ostringstream ostr;
    ostr << *current_thread << " attempted to wait on "
         << *this << " without holding " << _mutex;
    nassert_raise(ostr.str());
    _mutex._global_lock->release();
    return;
  }

  if (thread_cat.is_spam()) {
    thread_cat.spam()
      << *current_thread << " waiting on " << *this 
      << ", with timeout " << timeout << "\n";
  }

  nassertd(current_thread->_waiting_on_cvar == NULL &&
           current_thread->_waiting_on_cvar_full == NULL) {
  }
  current_thread->_waiting_on_cvar = this;
  
  _mutex.do_release();
  _impl.wait(timeout);  // temporarily releases _global_lock
  _mutex.do_acquire(current_thread);

  nassertd(current_thread->_waiting_on_cvar == this) {
  }
  current_thread->_waiting_on_cvar = NULL;

  if (thread_cat.is_spam()) {
    thread_cat.spam()
      << *current_thread << " awake on " << *this << "\n";
  }

  _mutex._global_lock->release();
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarDebug::notify
//       Access: Published
//  Description: Informs one of the other threads who are currently
//               blocked on wait() that the relevant condition has
//               changed.  If multiple threads are currently waiting,
//               at least one of them will be woken up, although there
//               is no way to predict which one.  It is possible that
//               more than one thread will be woken up.
//
//               The caller must be holding the mutex associated with
//               the condition variable before making this call, which
//               will not release the mutex.
//
//               If no threads are waiting, this is a no-op: the
//               notify event is lost.
////////////////////////////////////////////////////////////////////
void ConditionVarDebug::
notify() {
  _mutex._global_lock->acquire();

  Thread *current_thread = Thread::get_current_thread();

  if (!_mutex.do_debug_is_locked()) {
    ostringstream ostr;
    ostr << *current_thread << " attempted to notify "
         << *this << " without holding " << _mutex;
    nassert_raise(ostr.str());
    _mutex._global_lock->release();
    return;
  }

  if (thread_cat->is_spam()) {
    thread_cat.spam()
      << *current_thread << " notifying " << *this << "\n";
  }

  _impl.notify();
  _mutex._global_lock->release();
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarDebug::output
//       Access: Published, Virtual
//  Description: This method is declared virtual in ConditionVarDebug,
//               but non-virtual in ConditionVarDirect.
////////////////////////////////////////////////////////////////////
void ConditionVarDebug::
output(ostream &out) const {
  out << "ConditionVar " << (void *)this << " on " << _mutex;
}

#endif  // DEBUG_THREADS
