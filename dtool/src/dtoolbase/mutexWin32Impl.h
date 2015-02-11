// Filename: mutexWin32Impl.h
// Created by:  drose (07Feb06)
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

#ifndef MUTEXWIN32IMPL_H
#define MUTEXWIN32IMPL_H

#include "dtoolbase.h"
#include "selectThreadImpl.h"

#ifdef WIN32_VC
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>

////////////////////////////////////////////////////////////////////
//       Class : MutexWin32Impl
// Description : Uses Windows native calls to implement a mutex.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOL MutexWin32Impl {
public:
  MutexWin32Impl();
  INLINE ~MutexWin32Impl();

  INLINE void acquire();
  INLINE bool try_acquire();
  INLINE void release();

private:
  CRITICAL_SECTION _lock;
  friend class ConditionVarWin32Impl;
  friend class ConditionVarFullWin32Impl;
};

#include "mutexWin32Impl.I"

#endif  // WIN32_VC

#endif
