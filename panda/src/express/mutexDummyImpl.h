// Filename: mutexDummyImpl.h
// Created by:  drose (08Aug02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef MUTEXDUMMYIMPL_H
#define MUTEXDUMMYIMPL_H

#include "pandabase.h"
#include "selectIpcImpl.h"

#ifdef IPC_DUMMY_IMPL

#include "notify.h"

////////////////////////////////////////////////////////////////////
//       Class : MutexDummyImpl
// Description : A fake mutex implementation for single-threaded
//               applications that don't need any synchronization
//               control.  This does nothing but assert that the same
//               process does not try to grab the mutex twice.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS MutexDummyImpl {
public:
  INLINE MutexDummyImpl();
  INLINE ~MutexDummyImpl();

  INLINE void lock();
  INLINE void release();

private:
#ifndef NDEBUG
  int _lock_count;
#endif
};

#include "mutexDummyImpl.I"

#endif  // IPC_DUMMY_IMPL

#endif
