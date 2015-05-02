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
// mutex locks, as well as platform-independent thread constructs.

#ifdef _WIN32

// Windows case

// Locks are straightforward.
class _lock {
public:
  CRITICAL_SECTION _l;
  int _count;
};

#define LOCK _lock
#define INIT_LOCK(lock) { InitializeCriticalSection(&(lock)._l); (lock)._count = 0; }
#define ACQUIRE_LOCK(lock) { EnterCriticalSection(&(lock)._l); ++((lock)._count); }
#define RELEASE_LOCK(lock) { --((lock)._count); LeaveCriticalSection(&(lock)._l); }
#define DESTROY_LOCK(lock) DeleteCriticalSection(&(lock)._l)

/*
#define LOCK CRITICAL_SECTION
#define INIT_LOCK(lock) InitializeCriticalSection(&(lock))
#define ACQUIRE_LOCK(lock) EnterCriticalSection(&(lock))
#define RELEASE_LOCK(lock) LeaveCriticalSection(&(lock))
#define DESTROY_LOCK(lock) DeleteCriticalSection(&(lock))
*/

// Threads.
#define THREAD HANDLE
#define INIT_THREAD(thread) (thread) = NULL;
#define SPAWN_THREAD(thread, callback_function, this)                   \
  (thread) = CreateThread(NULL, 0, &win_ ## callback_function, (this), 0, NULL)
#define JOIN_THREAD(thread)                     \
  {                                             \
    assert((thread) != NULL);                   \
    WaitForSingleObject((thread), INFINITE);    \
    CloseHandle((thread));                      \
    (thread) = NULL;                            \
  }

// Declare this macro within your class declaration.  This implements
// the callback function wrapper necessary to hook into the above
// SPAWN_THREAD call.  The wrapper will in turn call the method
// function you provide.
#define THREAD_CALLBACK_DECLARATION(class, callback_function) \
  static DWORD WINAPI                                         \
  win_ ## callback_function(LPVOID data) {                    \
    ((class *)data)->callback_function();                     \
    return 0;                                                 \
  }


#else  // _WIN32

// Posix case
#include <pthread.h>

// We declare this to be a recursive lock, since we might make a
// request_ready call from within the API, which in turn is allowed to
// call back into the API.
#define LOCK pthread_mutex_t
#define INIT_LOCK(lock) {                                      \
    pthread_mutexattr_t attr;                                  \
    pthread_mutexattr_init(&attr);                             \
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE); \
    pthread_mutex_init(&(lock), &attr);                        \
    pthread_mutexattr_destroy(&attr);                          \
  }
#define ACQUIRE_LOCK(lock) pthread_mutex_lock(&(lock))
#define RELEASE_LOCK(lock) pthread_mutex_unlock(&(lock))
#define DESTROY_LOCK(lock) pthread_mutex_destroy(&(lock))

#define THREAD pthread_t
#define INIT_THREAD(thread) (thread) = 0;
#define SPAWN_THREAD(thread, callback_function, this)                   \
  {                                                                     \
    pthread_attr_t attr;                                                \
    pthread_attr_init(&attr);                                           \
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);                 \
    pthread_create(&(thread), &attr, &posix_ ## callback_function, (void *)(this)); \
    pthread_attr_destroy(&attr);                                        \
  }

#define JOIN_THREAD(thread)                             \
  {                                                     \
    assert((thread) != 0);                              \
    void *return_val;                                   \
    int success = pthread_join((thread), &return_val);  \
    (thread) = 0;                                       \
    if (success != 0) {                                 \
      nout << "Failed to join: " << success << "\n";    \
    } else {                                            \
      nout << "Successfully joined thread: " << return_val << "\n";     \
    }                                                   \
  }

// As above, declare this macro within your class declaration.
#define THREAD_CALLBACK_DECLARATION(class, callback_function) \
  static void *                                               \
  posix_ ## callback_function(void *data) {                   \
    ((class *)data)->callback_function();                     \
    return NULL;                                              \
  }

#endif  // _WIN32

#endif

