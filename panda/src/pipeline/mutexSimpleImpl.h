// Filename: mutexSimpleImpl.h
// Created by:  drose (19Jun07)
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

#ifndef MUTEXSIMPLEIMPL_H
#define MUTEXSIMPLEIMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#ifdef THREAD_SIMPLE_IMPL

#include "blockerSimple.h"
#include "threadSimpleImpl.h"

////////////////////////////////////////////////////////////////////
//       Class : MutexSimpleImpl
// Description : This is the mutex implementation for the simple,
//               simulated threads case.  It's designed to be as
//               lightweight as possible, of course.  This
//               implementation simply yields the thread when the
//               mutex would block.
//
//               We can't define this class in dtoolbase along with
//               the other mutex implementations, because this
//               implementation requires knowing about the
//               SimpleThreadManager.  This complicates the
//               MutexDirect and MutexDebug definitions (we have to
//               define a MutexImpl, for code before pipeline to
//               use--which maps to MutexDummyImpl--and a
//               MutexTrueImpl, for code after pipeline to use--which
//               maps to this class, MutexSimpleImpl).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PIPELINE MutexSimpleImpl : public BlockerSimple {
public:
  INLINE MutexSimpleImpl();
  INLINE ~MutexSimpleImpl();

  INLINE void lock();
  INLINE bool try_lock();
  INLINE void release();

private:
  void do_lock();
  void do_release();

  friend class ThreadSimpleManager;
};

#include "mutexSimpleImpl.I"

#endif  // THREAD_SIMPLE_IMPL

#endif
