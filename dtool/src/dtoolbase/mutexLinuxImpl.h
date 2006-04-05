// Filename: mutexLinuxImpl.h
// Created by:  drose (28Mar06)
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

#ifndef MUTEXLINUXIMPL_H
#define MUTEXLINUXIMPL_H

#include "dtoolbase.h"
#include "selectThreadImpl.h"

#ifdef THREAD_LINUX_IMPL

#include "numeric_types.h"

#undef MUTEX_DEFINES_TRYLOCK

////////////////////////////////////////////////////////////////////
//       Class : MutexLinuxImpl
// Description : Uses Linux threads to implement a mutex.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOL MutexLinuxImpl {
public:
  INLINE MutexLinuxImpl();
  INLINE ~MutexLinuxImpl();

  void lock();
  void release();

private:
  enum Mode {
    M_unlocked = 0,
    M_locked_no_waiters = 1,
    M_locked_with_waiters = 2,
  };

  PN_int32 _mode;
  friend class ConditionVarLinuxImpl;
};

#include "mutexLinuxImpl.I"

#endif  // THREAD_LINUX_IMPL

#endif
