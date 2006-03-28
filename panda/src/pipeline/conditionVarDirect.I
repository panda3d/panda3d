// Filename: conditionVarDirect.I
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


////////////////////////////////////////////////////////////////////
//     Function: ConditionVarDirect::Constructor
//       Access: Public
//  Description: You must pass in a Mutex to the condition variable
//               constructor.  This mutex may be shared by other
//               condition variables, if desired.  It is the caller's
//               responsibility to ensure the Mutex object does not
//               destruct during the lifetime of the condition
//               variable.
////////////////////////////////////////////////////////////////////
INLINE ConditionVarDirect::
ConditionVarDirect(MutexDirect &mutex) :
  _mutex(mutex),
  _impl(mutex._impl)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarDirect::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ConditionVarDirect::
~ConditionVarDirect() {
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarDirect::Copy Constructor
//       Access: Private
//  Description: Do not attempt to copy condition variables.
////////////////////////////////////////////////////////////////////
INLINE ConditionVarDirect::
ConditionVarDirect(const ConditionVarDirect &copy) : 
  _mutex(copy._mutex), 
  _impl(_mutex._impl)
{
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarDirect::Copy Assignment Operator
//       Access: Private
//  Description: Do not attempt to copy condition variables.
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarDirect::
operator = (const ConditionVarDirect &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarDirect::get_mutex
//       Access: Public
//  Description: Returns the mutex associated with this condition
//               variable.
////////////////////////////////////////////////////////////////////
INLINE MutexDirect &ConditionVarDirect::
get_mutex() const {
  return _mutex;
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarDirect::wait
//       Access: Public
//  Description: Waits on the condition.  The caller must already be
//               holding the lock associated with the condition
//               variable before calling this function.
//
//               wait() will release the lock, then go to sleep until
//               some other thread calls signal() on this condition
//               variable.  At that time at least one thread waiting
//               on the same ConditionVarDirect will grab the lock again,
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
INLINE void ConditionVarDirect::
wait() {
  _impl.wait();
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarDirect::signal
//       Access: Public
//  Description: Informs one of the other threads who are currently
//               blocked on wait() that the relevant condition has
//               changed.  If multiple threads are currently waiting,
//               at least one of them will be woken up, although there
//               is no way to predict which one.
//
//               The caller must be holding the mutex associated with
//               the condition variable before making this call, which
//               will not release the mutex.
//
//               If no threads are waiting, this is a no-op: the
//               signal is lost.
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarDirect::
signal() {
  _impl.signal();
}
