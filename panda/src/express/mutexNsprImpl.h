// Filename: mutexNsprImpl.h
// Created by:  drose (08Aug02)
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

#ifndef MUTEXNSPRIMPL_H
#define MUTEXNSPRIMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#ifdef THREAD_NSPR_IMPL

#include "notify.h"

#include <prlock.h>

////////////////////////////////////////////////////////////////////
//       Class : MutexNsprImpl
// Description : Uses NSPR to implement a mutex.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS MutexNsprImpl {
public:
  INLINE MutexNsprImpl();
  INLINE ~MutexNsprImpl();

  INLINE void lock();
  INLINE void release();

private:
  PRLock *_lock;
  friend class ConditionVarNsprImpl;
};

#include "mutexNsprImpl.I"

#endif  // THREAD_NSPR_IMPL

#endif
