// Filename: p3dConditionVar.cxx
// Created by:  drose (02Jul09)
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

#include "p3dConditionVar.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DConditionVar::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DConditionVar::
P3DConditionVar() {
#ifdef _WIN32
  InitializeCriticalSection(&_lock);

  // Create an auto-reset event.
  _event_signal = CreateEvent(NULL, false, false, NULL);

#else  // _WIN32
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
  int result = pthread_mutex_init(&_lock, &attr);
  pthread_mutexattr_destroy(&attr);
  assert(result == 0);

  result = pthread_cond_init(&_cvar, NULL);
  assert(result == 0);

#endif  // _WIN32
}

////////////////////////////////////////////////////////////////////
//     Function: P3DConditionVar::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DConditionVar::
~P3DConditionVar() {
#ifdef _WIN32
  DeleteCriticalSection(&_lock);
  CloseHandle(_event_signal);

#else  // _WIN32
  int result = pthread_mutex_destroy(&_lock);
  assert(result == 0);

  result = pthread_cond_destroy(&_cvar);
  assert(result == 0);

#endif  // _WIN32
}

////////////////////////////////////////////////////////////////////
//     Function: P3DConditionVar::acquire
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void P3DConditionVar::
acquire() {
#ifdef _WIN32
  EnterCriticalSection(&_lock);

#else  // _WIN32
  int result = pthread_mutex_lock(&_lock);
  assert(result == 0);

#endif  // _WIN32
}

////////////////////////////////////////////////////////////////////
//     Function: P3DConditionVar::wait
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void P3DConditionVar::
wait() {
#ifdef _WIN32
  LeaveCriticalSection(&_lock);

  DWORD result = WaitForSingleObject(_event_signal, INFINITE);
  assert(result == WAIT_OBJECT_0);

  EnterCriticalSection(&_lock);

#else  // _WIN32
  int result = pthread_cond_wait(&_cvar, &_mutex._lock);
  assert(result == 0);

#endif  // _WIN32
}

////////////////////////////////////////////////////////////////////
//     Function: P3DConditionVar::notify
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void P3DConditionVar::
notify() {
#ifdef _WIN32
  SetEvent(_event_signal);

#else  // _WIN32
  int result = pthread_cond_signal(&_cvar);
  assert(result == 0);

#endif  // _WIN32
}

////////////////////////////////////////////////////////////////////
//     Function: P3DConditionVar::release
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void P3DConditionVar::
release() {
#ifdef _WIN32
  LeaveCriticalSection(&_lock);

#else  // _WIN32
  int result = pthread_mutex_unlock(&_lock);
  assert(result == 0);

#endif  // _WIN32
}
