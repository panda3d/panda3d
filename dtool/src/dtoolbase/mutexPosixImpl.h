// Filename: mutexPosixImpl.h
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

#ifndef MUTEXPOSIXIMPL_H
#define MUTEXPOSIXIMPL_H

#include "dtoolbase.h"
#include "selectThreadImpl.h"

#ifdef HAVE_POSIX_THREADS

#include <pthread.h>
#include <errno.h>
#include <assert.h>

////////////////////////////////////////////////////////////////////
//       Class : MutexPosixImpl
// Description : Uses Posix threads to implement a mutex.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOL MutexPosixImpl {
public:
  INLINE MutexPosixImpl();
  INLINE ~MutexPosixImpl();

  INLINE void acquire();
  INLINE bool try_acquire();
  INLINE void release();

  INLINE pthread_mutex_t *get_posix_lock();

private:
  pthread_mutex_t _lock;
  friend class ConditionVarPosixImpl;
};

////////////////////////////////////////////////////////////////////
//       Class : ReMutexPosixImpl
// Description : Uses Posix threads to implement a reentrant mutex.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOL ReMutexPosixImpl {
public:
  INLINE ReMutexPosixImpl();
  INLINE ~ReMutexPosixImpl();

  INLINE void acquire();
  INLINE bool try_acquire();
  INLINE void release();

  INLINE pthread_mutex_t *get_posix_lock();

private:
  pthread_mutex_t _lock;
};

#include "mutexPosixImpl.I"

#endif  // HAVE_POSIX_THREADS

#endif
