// Filename: conditionVarSpinlockImpl.cxx
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

#include "selectThreadImpl.h"

#ifdef MUTEX_SPINLOCK

#include "conditionVarSpinlockImpl.h"

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarSpinlockImpl::wait
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void ConditionVarSpinlockImpl::
wait() {
  AtomicAdjust::Integer current = _event;
  _mutex.release();

  while (AtomicAdjust::get(_event) == current) {
  }

  _mutex.lock();
}

#endif  // MUTEX_SPINLOCK
