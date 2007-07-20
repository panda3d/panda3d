// Filename: reMutexSimpleImpl.h
// Created by:  drose (20Jun07)
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

#ifndef REMUTEXSIMPLEIMPL_H
#define REMUTEXSIMPLEIMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#ifdef THREAD_SIMPLE_IMPL

#include "blockerSimple.h"
#include "threadSimpleImpl.h"

////////////////////////////////////////////////////////////////////
//       Class : ReMutexSimpleImpl
// Description : This is the mutex implementation for the simple,
//               simulated threads case, for recursive mutexes.  See
//               MutexSimpleImple.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PIPELINE ReMutexSimpleImpl : public BlockerSimple {
public:
  INLINE ReMutexSimpleImpl();
  INLINE ~ReMutexSimpleImpl();

  INLINE void lock();
  INLINE bool try_lock();
  INLINE void release();

private:
  void do_lock();
  void do_release();

  ThreadSimpleImpl *_locking_thread;

  friend class ThreadSimpleManager;
};

#include "reMutexSimpleImpl.I"

#endif  // THREAD_SIMPLE_IMPL

#endif
