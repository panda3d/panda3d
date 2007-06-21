// Filename: conditionVarSimpleImpl.I
// Created by:  drose (19Jun07)
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
//     Function: ConditionVarSimpleImpl::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ConditionVarSimpleImpl::
ConditionVarSimpleImpl(MutexTrueImpl &mutex) : _mutex(mutex) {
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarSimpleImpl::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ConditionVarSimpleImpl::
~ConditionVarSimpleImpl() {
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarSimpleImpl::signal
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarSimpleImpl::
signal() {
  if (_flags & F_has_waiters) {
    do_signal();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarSimpleImpl::signal_all
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarSimpleImpl::
signal_all() {
  if (_flags & F_has_waiters) {
    do_signal_all();
  }
}
