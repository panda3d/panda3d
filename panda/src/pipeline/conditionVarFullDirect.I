// Filename: conditionVarFullDirect.I
// Created by:  drose (28Aug06)
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
//       Access: Published
//  Description: Returns the mutex associated with this condition
//               variable.
////////////////////////////////////////////////////////////////////
INLINE MutexDirect &ConditionVarFullDirect::
get_mutex() const {
  return _mutex;
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarFullDirect::wait
//       Access: Published
//  Description: Waits on the condition.  The caller must already be
//               holding the lock associated with the condition
//               variable before calling this function.
//
//               wait() will release the lock, then go to sleep until
//               some other thread calls notify() on this condition
//               variable.  At that time at least one thread waiting
//               on the same ConditionVarFullDirect will grab the lock again,
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
INLINE void ConditionVarFullDirect::
wait() {
  TAU_PROFILE("ConditionVarFullDirect::wait()", " ", TAU_USER);
  _impl.wait();
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarFullDirect::wait
//       Access: Published
//  Description: Waits on the condition, with a timeout.  The function
//               will return when the condition variable is notified,
//               or the timeout occurs.  There is no way to directly
//               tell which happened, and it is possible that neither
//               in fact happened (spurious wakeups are possible).
//
//               See wait() with no parameters for more.
////////////////////////////////////////////////////////////////////
void ConditionVarFullDirect::
wait(double timeout) {
  TAU_PROFILE("ConditionVarFullDirect::wait(double)", " ", TAU_USER);
  _impl.wait(timeout);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarFullDirect::notify
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
//               notify is lost.
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarFullDirect::
notify() {
  TAU_PROFILE("ConditionVarFullDirect::notify()", " ", TAU_USER);
  _impl.notify();
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarFullDirect::notify_all
//       Access: Published
//  Description: Informs all of the other threads who are currently
//               blocked on wait() that the relevant condition has
//               changed.
//
//               The caller must be holding the mutex associated with
//               the condition variable before making this call, which
//               will not release the mutex.
//
//               If no threads are waiting, this is a no-op: the
//               notify event is lost.
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarFullDirect::
notify_all() {
  TAU_PROFILE("ConditionVarFullDirect::notify()", " ", TAU_USER);
  _impl.notify_all();
}
