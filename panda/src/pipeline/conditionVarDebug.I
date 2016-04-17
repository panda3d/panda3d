/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file conditionVarDebug.I
 * @author drose
 * @date 2006-02-13
 */

/**
 * Do not attempt to copy condition variables.
 */
INLINE ConditionVarDebug::
ConditionVarDebug(const ConditionVarDebug &copy) :
  _mutex(copy._mutex),
  _impl(*_mutex._global_lock)
{
  nassertv(false);
}

/**
 * Do not attempt to copy condition variables.
 */
INLINE void ConditionVarDebug::
operator = (const ConditionVarDebug &copy) {
  nassertv(false);
}

/**
 * Returns the mutex associated with this condition variable.
 */
INLINE MutexDebug &ConditionVarDebug::
get_mutex() const {
  return _mutex;
}
