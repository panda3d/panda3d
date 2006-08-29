// Filename: conditionVarFullDirect.I
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
//     Function: ConditionVarFullDirect::Constructor
//       Access: Public
//  Description: You must pass in a Mutex to the condition variable
//               constructor.  This mutex may be shared by other
//               condition variables, if desired.  It is the caller's
//               responsibility to ensure the Mutex object does not
//               destruct during the lifetime of the condition
//               variable.
////////////////////////////////////////////////////////////////////
INLINE ConditionVarFullDirect::
ConditionVarFullDirect(MutexDirect &mutex) :
  _mutex(mutex),
  _impl(mutex._impl)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarFullDirect::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ConditionVarFullDirect::
~ConditionVarFullDirect() {
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarFullDirect::Copy Constructor
//       Access: Private
//  Description: Do not attempt to copy condition variables.
////////////////////////////////////////////////////////////////////
INLINE ConditionVarFullDirect::
ConditionVarFullDirect(const ConditionVarFullDirect &copy) : 
  _mutex(copy._mutex), 
  _impl(_mutex._impl)
{
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarFullDirect::Copy Assignment Operator
//       Access: Private
//  Description: Do not attempt to copy condition variables.
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarFullDirect::
operator = (const ConditionVarFullDirect &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarFullDirect::get_mutex
//       Access: Public
//  Description: Returns the mutex associated with this condition
//               variable.
////////////////////////////////////////////////////////////////////
INLINE MutexDirect &ConditionVarFullDirect::
get_mutex() const {
  return _mutex;
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarFullDirect::wait
//       Access: Public
//  Description: Waits on the condition.  The caller must already be
//               holding the lock associated with the condition
//               variable before calling this function.
//
//               wait() will release the lock, then go to sleep until
//               some other thread calls signal() on this condition
//               variable.  At that time at least one thread waiting
//               on the same ConditionVarFullDirect will grab the lock again,
//               and then return from wait().
//
//               It is possible that wait() will return even if no one
//               has called signal().  It is the responsibility of the
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
INLINE void ConditionVarFullDirect::
wait() {
  TAU_PROFILE("ConditionVarFullDirect::wait()", " ", TAU_USER);
  _impl.wait();
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarFullDirect::signal
//       Access: Public
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
//               signal is lost.
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarFullDirect::
signal() {
  TAU_PROFILE("ConditionVarFullDirect::signal()", " ", TAU_USER);
  _impl.signal();
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarFullDirect::signal_all
//       Access: Public
//  Description: Informs all of the other threads who are currently
//               blocked on wait() that the relevant condition has
//               changed.
//
//               The caller must be holding the mutex associated with
//               the condition variable before making this call, which
//               will not release the mutex.
//
//               If no threads are waiting, this is a no-op: the
//               signal is lost.
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarFullDirect::
signal_all() {
  TAU_PROFILE("ConditionVarFullDirect::signal()", " ", TAU_USER);
  _impl.signal_all();
}
