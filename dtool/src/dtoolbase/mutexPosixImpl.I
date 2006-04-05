// Filename: mutexPosixImpl.I
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
//     Function: MutexPosixImpl::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MutexPosixImpl::
MutexPosixImpl() {
  TAU_PROFILE("MutexPosixImpl::MutexPosixImpl", " ", TAU_USER);
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
  int result = pthread_mutex_init(&_lock, &attr);
  pthread_mutexattr_destroy(&attr);
  assert(result == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: MutexPosixImpl::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MutexPosixImpl::
~MutexPosixImpl() {
  TAU_PROFILE("MutexPosixImpl::~MutexPosixImpl", " ", TAU_USER);
  int result = pthread_mutex_destroy(&_lock);
  assert(result == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: MutexPosixImpl::lock
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void MutexPosixImpl::
lock() {
  TAU_PROFILE("MutexPosixImpl::lock", " ", TAU_USER);
  int result = pthread_mutex_lock(&_lock);
  assert(result == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: MutexPosixImpl::try_lock
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool MutexPosixImpl::
try_lock() {
  TAU_PROFILE("MutexPosixImpl::try_lock", " ", TAU_USER);
  int result = pthread_mutex_trylock(&_lock);
  assert(result == 0 || result == EBUSY);
  return (result == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: MutexPosixImpl::release
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void MutexPosixImpl::
release() {
  TAU_PROFILE("MutexPosixImpl::release", " ", TAU_USER);
  int result = pthread_mutex_unlock(&_lock);
  assert(result == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexPosixImpl::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ReMutexPosixImpl::
ReMutexPosixImpl() {
  TAU_PROFILE("ReMutexPosixImpl::ReMutexPosixImpl", " ", TAU_USER);
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
  int result = pthread_mutex_init(&_lock, &attr);
  pthread_mutexattr_destroy(&attr);
  assert(result == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexPosixImpl::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ReMutexPosixImpl::
~ReMutexPosixImpl() {
  TAU_PROFILE("ReMutexPosixImpl::~ReMutexPosixImpl", " ", TAU_USER);
  int result = pthread_mutex_destroy(&_lock);
  assert(result == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexPosixImpl::lock
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ReMutexPosixImpl::
lock() {
  TAU_PROFILE("ReMutexPosixImpl::lock", " ", TAU_USER);
  int result = pthread_mutex_lock(&_lock);
  assert(result == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexPosixImpl::try_lock
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool ReMutexPosixImpl::
try_lock() {
  TAU_PROFILE("ReMutexPosixImpl::try_lock", " ", TAU_USER);
  int result = pthread_mutex_trylock(&_lock);
  assert(result == 0 || result == EBUSY);
  return (result == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexPosixImpl::release
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ReMutexPosixImpl::
release() {
  TAU_PROFILE("ReMutexPosixImpl::release", " ", TAU_USER);
  int result = pthread_mutex_unlock(&_lock);
  assert(result == 0);
}
