/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file conditionVarSpinlockImpl.I
 * @author drose
 * @date 2006-04-11
 */

/**
 *
 */
INLINE ConditionVarSpinlockImpl::
ConditionVarSpinlockImpl(MutexSpinlockImpl &mutex) : _mutex(mutex) {
  _event = 0;
}

/**
 *
 */
INLINE ConditionVarSpinlockImpl::
~ConditionVarSpinlockImpl() {
}

/**
 *
 */
INLINE void ConditionVarSpinlockImpl::
notify() {
  // This will wake up all waiters on the lock.  But that's allowed.
  AtomicAdjust::inc(_event);
}

/**
 *
 */
INLINE void ConditionVarSpinlockImpl::
notify_all() {
  AtomicAdjust::inc(_event);
}
