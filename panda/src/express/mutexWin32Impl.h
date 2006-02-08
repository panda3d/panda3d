// Filename: mutexWin32Impl.h
// Created by:  drose (07Feb06)
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

#ifndef MUTEXWIN32IMPL_H
#define MUTEXWIN32IMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#ifdef THREAD_WIN32_IMPL

#include "notify.h"

#include <windows.h>

////////////////////////////////////////////////////////////////////
//       Class : MutexWin32Impl
// Description : Uses Windows native calls to implement a mutex.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS MutexWin32Impl {
public:
  INLINE MutexWin32Impl();
  INLINE ~MutexWin32Impl();

  INLINE void lock();
  INLINE void release();

private:
  CRITICAL_SECTION _lock;
  friend class ConditionVarWin32Impl;
};

#include "mutexWin32Impl.I"

#endif  // THREAD_WIN32_IMPL

#endif
