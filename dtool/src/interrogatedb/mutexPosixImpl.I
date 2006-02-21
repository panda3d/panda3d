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
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
  int result = pthread_mutex_init(&_lock, &attr);
  pthread_mutexattr_destroy(&attr);
  nassertv(result == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: MutexPosixImpl::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MutexPosixImpl::
~MutexPosixImpl() {
  int result = pthread_mutex_destroy(&_lock);
  nassertv(result == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: MutexPosixImpl::lock
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void MutexPosixImpl::
lock() {
  int result = pthread_mutex_lock(&_lock);
  nassertv(result == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: MutexPosixImpl::try_lock
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool MutexPosixImpl::
try_lock() {
  int result = pthread_mutex_trylock(&_lock);
  nassertr(result == 0 || result == EBUSY, false);
  return (result == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: MutexPosixImpl::release
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void MutexPosixImpl::
release() {
  int result = pthread_mutex_unlock(&_lock);
  nassertv(result == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexPosixImpl::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ReMutexPosixImpl::
ReMutexPosixImpl() {
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
  int result = pthread_mutex_init(&_lock, &attr);
  pthread_mutexattr_destroy(&attr);
  nassertv(result == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexPosixImpl::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ReMutexPosixImpl::
~ReMutexPosixImpl() {
  int result = pthread_mutex_destroy(&_lock);
  nassertv(result == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexPosixImpl::lock
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ReMutexPosixImpl::
lock() {
  int result = pthread_mutex_lock(&_lock);
  nassertv(result == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexPosixImpl::try_lock
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool ReMutexPosixImpl::
try_lock() {
  int result = pthread_mutex_trylock(&_lock);
  nassertr(result == 0 || result == EBUSY, false);
  return (result == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexPosixImpl::release
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ReMutexPosixImpl::
release() {
  int result = pthread_mutex_unlock(&_lock);
  nassertv(result == 0);
}
