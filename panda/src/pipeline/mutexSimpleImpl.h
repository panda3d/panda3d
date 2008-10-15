// Filename: mutexSimpleImpl.h
// Created by:  drose (19Jun07)
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

  INLINE void acquire();
  INLINE bool try_acquire();
  INLINE void release();
  INLINE void release_quietly();

private:
  void do_acquire();
  void do_release();
  void do_release_quietly();

  friend class ThreadSimpleManager;
};

#include "mutexSimpleImpl.I"

#endif  // THREAD_SIMPLE_IMPL

#endif
