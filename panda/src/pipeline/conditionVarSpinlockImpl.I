// Filename: conditionVarSpinlockImpl.I
// Created by:  drose (11Apr06)
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
//     Function: ConditionVarSpinlockImpl::signal
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarSpinlockImpl::
signal() {
  // This will wake up all waiters on the lock.  But that's allowed.
  AtomicAdjust::inc(_event);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarSpinlockImpl::signal_all
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarSpinlockImpl::
signal_all() {
  AtomicAdjust::inc(_event);
}
