// Filename: conditionVarPosixImpl.I
// Created by:  drose (10Feb06)
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
//     Function: ConditionVarPosixImpl::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ConditionVarPosixImpl::
ConditionVarPosixImpl(MutexPosixImpl &mutex) :
  _mutex(mutex)
{
  TAU_PROFILE("ConditionVarPosixImpl::ConditionVarPosixImpl()", " ", TAU_USER);
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
  TAU_PROFILE("ConditionVarPosixImpl::~ConditionVarPosixImpl()", " ", TAU_USER);
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
  TAU_PROFILE("ConditionVarPosixImpl::wait()", " ", TAU_USER);
  int result = pthread_cond_wait(&_cvar, &_mutex._lock);
#ifndef NDEBUG
  if (result != 0) {
    pipeline_cat.error()
      << "Unexpected error " << result << " from pthread_cond_wait()\n";
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarPosixImpl::notify
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarPosixImpl::
notify() {
  TAU_PROFILE("ConditionVarPosixImpl::notify()", " ", TAU_USER);
  int result = pthread_cond_signal(&_cvar);
  nassertv(result == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarPosixImpl::notify_all
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarPosixImpl::
notify_all() {
  TAU_PROFILE("ConditionVarPosixImpl::notify()", " ", TAU_USER);
  int result = pthread_cond_broadcast(&_cvar);
  nassertv(result == 0);
}
