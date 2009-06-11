// Filename: p3d_lock.h
// Created by:  drose (05Jun09)
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

#ifndef P3D_LOCK_H
#define P3D_LOCK_H

// Provides some simple macros that implement platform-independet
// mutex locks.

#ifdef _WIN32

// Windows case
#define LOCK CRITICAL_SECTION
#define INIT_LOCK(lock) InitializeCriticalSection(&(lock))
#define ACQUIRE_LOCK(lock) EnterCriticalSection(&(lock))
#define RELEASE_LOCK(lock) LeaveCriticalSection(&(lock))
#define DESTROY_LOCK(lock) DeleteCriticalSection(&(lock))

#else  // _WIN32

// Posix case
#include <pthread.h>

#define LOCK pthread_mutex_t
#define INIT_LOCK(lock) { \
    pthread_mutexattr_t attr; \
    pthread_mutexattr_init(&attr); \
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL); \
    int result = pthread_mutex_init(&(lock), &attr);        \
    pthread_mutexattr_destroy(&attr); \
  }
#define ACQUIRE_LOCK(lock) pthread_mutex_lock(&(lock))
#define RELEASE_LOCK(lock) pthread_mutex_unlock(&(lock))
#define DESTROY_LOCK(lock) pthread_mutex_destroy(&(lock))

#endif  // _WIN32

#endif

