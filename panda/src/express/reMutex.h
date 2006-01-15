// Filename: reMutex.h
// Created by:  drose (15Jan06)
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

#ifndef REMUTEX_H
#define REMUTEX_H

#include "pandabase.h"
#include "pmutex.h"
#include "conditionVar.h"
#include "mutexHolder.h"
#include "thread.h"


////////////////////////////////////////////////////////////////////
//       Class : ReMutex
// Description : A reentrant mutex.  This kind of mutex can be locked
//               again by the thread that already holds it, without
//               deadlock.  The thread must eventually release the
//               mutex the same number of times it locked it.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS ReMutex {
public:
  INLINE ReMutex();
  INLINE ~ReMutex();
private:
  INLINE ReMutex(const ReMutex &copy);
  INLINE void operator = (const ReMutex &copy);

public:
  INLINE void lock() const;
  INLINE void release() const;

  INLINE bool is_locked() const;
  INLINE int get_lock_count() const;

private:
  void do_lock();
  void do_release();

  Mutex _mutex;
  ConditionVar _cvar;
  Thread *_locking_thread;
  int _lock_count;
};

#include "reMutex.I"

#endif
