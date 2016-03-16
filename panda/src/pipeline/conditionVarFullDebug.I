/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file conditionVarFullDebug.I
 * @author drose
 * @date 2006-08-28
 */

/**
 * Do not attempt to copy condition variables.
 */
INLINE ConditionVarFullDebug::
ConditionVarFullDebug(const ConditionVarFullDebug &copy) :
  _mutex(copy._mutex),
  _impl(*_mutex._global_lock)
{
  nassertv(false);
}

/**
 * Do not attempt to copy condition variables.
 */
INLINE void ConditionVarFullDebug::
operator = (const ConditionVarFullDebug &copy) {
  nassertv(false);
}

/**
 * Returns the mutex associated with this condition variable.
 */
INLINE MutexDebug &ConditionVarFullDebug::
get_mutex() const {
  return _mutex;
}
