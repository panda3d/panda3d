// Filename: conditionVarNsprImpl.I
// Created by:  drose (09Aug02)
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
//     Function: ConditionVarNsprImpl::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ConditionVarNsprImpl::
ConditionVarNsprImpl(MutexNsprImpl &mutex) {
  _cvar = PR_NewCondVar(mutex._lock);
  nassertv(_cvar != (PRCondVar *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarNsprImpl::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ConditionVarNsprImpl::
~ConditionVarNsprImpl() {
  PR_DestroyCondVar(_cvar);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarNsprImpl::wait
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarNsprImpl::
wait() {
  int status = PR_WaitCondVar(_cvar, PR_INTERVAL_NO_TIMEOUT);
  nassertv(status == PR_SUCCESS);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarNsprImpl::signal
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarNsprImpl::
signal() {
  int status = PR_NotifyCondVar(_cvar);
  nassertv(status == PR_SUCCESS);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarNsprImpl::signal_all
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarNsprImpl::
signal_all() {
  int status = PR_NotifyAllCondVar(_cvar);
  nassertv(status == PR_SUCCESS);
}
