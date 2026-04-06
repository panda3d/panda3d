/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file conditionVarDirect.I
 * @author drose
 * @date 2006-02-13
 */

/**
 * You must pass in a Mutex to the condition variable constructor.  This mutex
 * may be shared by other condition variables, if desired.  It is the caller's
 * responsibility to ensure the Mutex object does not destruct during the
 * lifetime of the condition variable.
 */
INLINE ConditionVarDirect::
ConditionVarDirect(MutexDirect &mutex) :
  _mutex(mutex),
  _impl(mutex._impl)
{
}

/**
 * Returns the mutex associated with this condition variable.
 */
INLINE MutexDirect &ConditionVarDirect::
get_mutex() const {
  return _mutex;
}

/**
 * Waits on the condition.  The caller must already be holding the lock
 * associated with the condition variable before calling this function.
 *
 * wait() will release the lock, then go to sleep until some other thread
 * calls notify() on this condition variable.  At that time at least one
 * thread waiting on the same ConditionVarDirect will grab the lock again, and
 * then return from wait().
 *
 * It is possible that wait() will return even if no one has called notify().
 * It is the responsibility of the calling process to verify the condition on
 * return from wait, and possibly loop back to wait again if necessary.
 *
 * Note the semantics of a condition variable: the mutex must be held before
 * wait() is called, and it will still be held when wait() returns.  However,
 * it will be temporarily released during the wait() call itself.
 */
INLINE void ConditionVarDirect::
wait() {
  TAU_PROFILE("ConditionVarDirect::wait()", " ", TAU_USER);
  _impl.wait();
}

/**
 * Waits on the condition, with a timeout.  The function will return when the
 * condition variable is notified, or the timeout occurs.  There is no way to
 * directly tell which happened, and it is possible that neither in fact
 * happened (spurious wakeups are possible).
 *
 * See wait() with no parameters for more.
 */
void ConditionVarDirect::
wait(double timeout) {
  TAU_PROFILE("ConditionVarDirect::wait(double)", " ", TAU_USER);
  _impl.wait(timeout);
}

/**
 * Informs one of the other threads who are currently blocked on wait() that
 * the relevant condition has changed.  If multiple threads are currently
 * waiting, at least one of them will be woken up, although there is no way to
 * predict which one.  It is possible that more than one thread will be woken
 * up.
 *
 * If no threads are waiting, this is a no-op: the notify event is lost.
 */
INLINE void ConditionVarDirect::
notify() {
  TAU_PROFILE("ConditionVarDirect::notify()", " ", TAU_USER);
  _impl.notify();
}

/**
 * Informs all of the other threads who are currently blocked on wait() that
 * the relevant condition has changed.
 *
 * If no threads are waiting, this is a no-op: the notify event is lost.
 */
INLINE void ConditionVarDirect::
notify_all() {
  TAU_PROFILE("ConditionVarDirect::notify()", " ", TAU_USER);
  _impl.notify_all();
}
