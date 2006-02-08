// Filename: pmutex.h
// Created by:  cary (16Sep98)
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

#ifndef PMUTEX_H
#define PMUTEX_H

#include "pandabase.h"
#include "mutexImpl.h"

class Thread;

////////////////////////////////////////////////////////////////////
//       Class : Mutex
// Description : A standard mutex, or mutual exclusion lock.  Only one
//               thread can hold ("lock") a mutex at any given time;
//               other threads trying to grab the mutex will block
//               until the holding thread releases it.
//
//               The standard mutex is not reentrant: a thread may not
//               attempt to lock it twice.  Although this may happen
//               to work on some platforms (e.g. Win32), it will not
//               work on all platforms; on some platforms, a thread
//               can deadlock itself by attempting to lock the same
//               mutex twice.  If your code requires a reentrant
//               mutex, use the ReMutex class instead.  When this code
//               is compiled with CHECK_REENTRANT_MUTEX true, then it
//               will enable assertion checks to ensure that a
//               particular thread doesn't try to lock the same mutex
//               twice.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS Mutex {
public:
  INLINE Mutex();
  INLINE ~Mutex();
private:
  INLINE Mutex(const Mutex &copy);
  INLINE void operator = (const Mutex &copy);

public:
  INLINE void lock() const;
  INLINE void release() const;

#ifdef CHECK_REENTRANT_MUTEX
  bool debug_is_locked() const;
#else
  INLINE bool debug_is_locked() const;
#endif

private:
  void do_lock();
  void do_release();

private:
  MutexImpl _impl;

#ifdef CHECK_REENTRANT_MUTEX
  // Make sure that ordinary mutexes are not locked reentrantly
  // (that's what a ReMutex is for).
  Thread *_locking_thread;
#endif

  friend class ConditionVar;
};

#include "pmutex.I"

#endif
