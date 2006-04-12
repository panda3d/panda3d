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

#include "dtoolbase.h"
#include "selectThreadImpl.h"

#ifdef HAVE_NSPR

#include <prlock.h>
#include <prmon.h>

////////////////////////////////////////////////////////////////////
//       Class : MutexNsprImpl
// Description : Uses NSPR to implement a mutex.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOL MutexNsprImpl {
public:
  INLINE MutexNsprImpl();
  INLINE ~MutexNsprImpl();

  INLINE void lock();
  INLINE bool try_lock();
  INLINE void release();

private:
  PRLock *_lock;
  friend class ConditionVarNsprImpl;
};

////////////////////////////////////////////////////////////////////
//       Class : ReMutexNsprImpl
// Description : Uses NSPR to implement a reMutex.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOL ReMutexNsprImpl {
public:
  INLINE ReMutexNsprImpl();
  INLINE ~ReMutexNsprImpl();

  INLINE void lock();
  INLINE bool try_lock();
  INLINE void release();

private:
  PRMonitor *_monitor;
};

#include "mutexNsprImpl.I"

#endif  // HAVE_NSPR

#endif
