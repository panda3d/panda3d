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
#define LOCK CRITICAL_SECTION
#define INIT_LOCK(lock) InitializeCriticalSection(&(lock))
#define ACQUIRE_LOCK(lock) EnterCriticalSection(&(lock))
#define RELEASE_LOCK(lock) LeaveCriticalSection(&(lock))
#define DESTROY_LOCK(lock) DeleteCriticalSection(&(lock))
#endif

#endif

