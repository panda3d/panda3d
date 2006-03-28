// Filename: conditionVarPosixImpl.I
// Created by:  drose (10Feb06)
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
//     Function: ConditionVarPosixImpl::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ConditionVarPosixImpl::
ConditionVarPosixImpl(MutexPosixImpl &mutex) :
  _mutex(mutex)
{
  int result = pthread_cond_init(&_cvar, NULL);
  nassertv(result == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarPosixImpl::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ConditionVarPosixImpl::
~ConditionVarPosixImpl() {
  int result = pthread_cond_destroy(&_cvar);
  nassertv(result == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarPosixImpl::wait
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarPosixImpl::
wait() {
  int result = pthread_cond_wait(&_cvar, &_mutex._lock);
  nassertv(result == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarPosixImpl::signal
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarPosixImpl::
signal() {
  int result = pthread_cond_signal(&_cvar);
  nassertv(result == 0);
}
