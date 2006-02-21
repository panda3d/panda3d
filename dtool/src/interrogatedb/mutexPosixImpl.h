// Filename: mutexPosixImpl.h
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

#ifndef MUTEXPOSIXIMPL_H
#define MUTEXPOSIXIMPL_H

#include "dtoolbase.h"
#include "selectThreadImpl.h"

#ifdef THREAD_POSIX_IMPL

#include "notify.h"

#include <pthread.h>
#include <errno.h>

#define MUTEX_DEFINES_TRYLOCK 1

////////////////////////////////////////////////////////////////////
//       Class : MutexPosixImpl
// Description : Uses Posix threads to implement a mutex.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOLCONFIG MutexPosixImpl {
public:
  INLINE MutexPosixImpl();
  INLINE ~MutexPosixImpl();

  INLINE void lock();
  INLINE bool try_lock();
  INLINE void release();

private:
  pthread_mutex_t _lock;
  friend class ConditionVarPosixImpl;
};

////////////////////////////////////////////////////////////////////
//       Class : ReMutexPosixImpl
// Description : Uses Posix threads to implement a reentrant mutex.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOLCONFIG ReMutexPosixImpl {
public:
  INLINE ReMutexPosixImpl();
  INLINE ~ReMutexPosixImpl();

  INLINE void lock();
  INLINE bool try_lock();
  INLINE void release();

private:
  pthread_mutex_t _lock;
};

#include "mutexPosixImpl.I"

#endif  // THREAD_POSIX_IMPL

#endif
