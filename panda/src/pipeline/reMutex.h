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
#include "mutexDebug.h"
#include "reMutexDirect.h"

////////////////////////////////////////////////////////////////////
//       Class : ReMutex
// Description : A reentrant mutex.  This kind of mutex can be locked
//               more than once by the thread that already holds it,
//               without deadlock.  The thread must eventually release
//               the mutex the same number of times it locked it.
//
//               This class inherits its implementation either from
//               MutexDebug or ReMutexDirect, depending on the
//               definition of DEBUG_THREADS.
////////////////////////////////////////////////////////////////////
#ifdef DEBUG_THREADS
class EXPCL_PANDA_PIPELINE ReMutex : public MutexDebug
#else
class EXPCL_PANDA_PIPELINE ReMutex : public ReMutexDirect
#endif  // DEBUG_THREADS
{
PUBLISHED:
  INLINE ReMutex();
public:
  INLINE ReMutex(const char *name);
PUBLISHED:
  INLINE ReMutex(const string &name);
  INLINE ~ReMutex();
private:
  INLINE ReMutex(const ReMutex &copy);
  INLINE void operator = (const ReMutex &copy);
};

#include "reMutex.I"

#endif
