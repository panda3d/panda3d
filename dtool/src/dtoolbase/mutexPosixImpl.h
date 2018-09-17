/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mutexPosixImpl.h
 * @author drose
 * @date 2006-02-10
 */

#ifndef MUTEXPOSIXIMPL_H
#define MUTEXPOSIXIMPL_H

#include "dtoolbase.h"
#include "selectThreadImpl.h"

#ifdef HAVE_POSIX_THREADS

#include <pthread.h>
#include <errno.h>
#include <assert.h>

/**
 * Uses Posix threads to implement a mutex.
 */
class EXPCL_DTOOL_DTOOLBASE MutexPosixImpl {
public:
  constexpr MutexPosixImpl() noexcept;
  MutexPosixImpl(const MutexPosixImpl &copy) = delete;
  INLINE ~MutexPosixImpl();

  MutexPosixImpl &operator = (const MutexPosixImpl &copy) = delete;

public:
  INLINE void lock();
  INLINE bool try_lock();
  INLINE void unlock();

private:
  pthread_mutex_t _lock;
  friend class ConditionVarPosixImpl;
};

/**
 * Uses Posix threads to implement a reentrant mutex.
 */
class EXPCL_DTOOL_DTOOLBASE ReMutexPosixImpl {
public:
#ifdef PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
  constexpr ReMutexPosixImpl() noexcept;
#else
  INLINE ReMutexPosixImpl();
#endif
  ReMutexPosixImpl(const ReMutexPosixImpl &copy) = delete;
  INLINE ~ReMutexPosixImpl();

  ReMutexPosixImpl &operator = (const ReMutexPosixImpl &copy) = delete;

public:
  INLINE void lock();
  INLINE bool try_lock();
  INLINE void unlock();

private:
  pthread_mutex_t _lock;
};

#include "mutexPosixImpl.I"

#endif  // HAVE_POSIX_THREADS

#endif
