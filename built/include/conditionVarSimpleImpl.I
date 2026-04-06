/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file conditionVarSimpleImpl.I
 * @author drose
 * @date 2007-06-19
 */

/**
 *
 */
INLINE ConditionVarSimpleImpl::
ConditionVarSimpleImpl(MutexTrueImpl &mutex) : _mutex(mutex) {
}

/**
 *
 */
INLINE ConditionVarSimpleImpl::
~ConditionVarSimpleImpl() {
}

/**
 *
 */
INLINE void ConditionVarSimpleImpl::
notify() {
  if (_flags & F_has_waiters) {
    do_notify();
  }
}

/**
 *
 */
INLINE void ConditionVarSimpleImpl::
notify_all() {
  if (_flags & F_has_waiters) {
    do_notify_all();
  }
}
