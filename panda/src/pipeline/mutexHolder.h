// Filename: mutexHolder.h
// Created by:  drose (09Aug02)
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

#ifndef MUTEXHOLDER_H
#define MUTEXHOLDER_H

#include "pandabase.h"
#include "pmutex.h"

////////////////////////////////////////////////////////////////////
//       Class : MutexHolder
// Description : A lightweight C++ object whose constructor calls
//               lock() and whose destructor calls release() on a
//               mutex.  It is a C++ convenience wrapper to call
//               release() automatically when a block exits (for
//               instance, on return).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA MutexHolder {
public:
  INLINE MutexHolder(const Mutex &mutex);
  INLINE MutexHolder(Mutex *&mutex);
  INLINE ~MutexHolder();
private:
  INLINE MutexHolder(const MutexHolder &copy);
  INLINE void operator = (const MutexHolder &copy);

private:
  // If HAVE_THREADS is defined, the Mutex class implements an actual
  // mutex object of some kind.  If HAVE_THREADS is not defined, this
  // will be a MutexDummyImpl, which does nothing much anyway, so we
  // might as well not even store a pointer to one--but MutexDummyImpl
  // does perform some circularity testing in the case that NDEBUG is
  // not defined, so we go ahead and store a pointer in that case too.
#if defined(HAVE_THREADS) || !defined(NDEBUG)
  const Mutex *_mutex;
#endif
};

#include "mutexHolder.I"

#endif
