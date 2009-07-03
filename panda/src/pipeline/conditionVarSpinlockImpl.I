// Filename: conditionVarSpinlockImpl.I
// Created by:  drose (11Apr06)
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
//     Function: ConditionVarSpinlockImpl::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ConditionVarSpinlockImpl::
ConditionVarSpinlockImpl(MutexSpinlockImpl &mutex) : _mutex(mutex) {
  _event = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarSpinlockImpl::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ConditionVarSpinlockImpl::
~ConditionVarSpinlockImpl() {
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarSpinlockImpl::notify
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarSpinlockImpl::
notify() {
  // This will wake up all waiters on the lock.  But that's allowed.
  AtomicAdjust::inc(_event);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarSpinlockImpl::notify_all
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarSpinlockImpl::
notify_all() {
  AtomicAdjust::inc(_event);
}
