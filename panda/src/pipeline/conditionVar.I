/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file conditionVar.I
 * @author drose
 * @date 2002-08-09
 */

/**
 * You must pass in a Mutex to the condition variable constructor.  This mutex
 * may be shared by other condition variables, if desired.  It is the caller's
 * responsibility to ensure the Mutex object does not destruct during the
 * lifetime of the condition variable.
 */
INLINE ConditionVar::
ConditionVar(Mutex &mutex) :
#ifdef DEBUG_THREADS
  ConditionVarDebug(mutex)
#else
  ConditionVarDirect(mutex)
#endif  // DEBUG_THREADS
{
}

/**
 * Returns the mutex associated with this condition variable.
 */
INLINE Mutex &ConditionVar::
get_mutex() const {
#ifdef DEBUG_THREADS
  return (Mutex &)ConditionVarDebug::get_mutex();
#else
  return (Mutex &)ConditionVarDirect::get_mutex();
#endif  // DEBUG_THREADS
}
