// Filename: mutexHolder.h
// Created by:  drose (09Aug02)
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
class EXPCL_PANDAEXPRESS MutexHolder {
public:
  INLINE MutexHolder(const Mutex &mutex);
  INLINE MutexHolder(Mutex *&mutex);
  INLINE ~MutexHolder();
private:
  INLINE MutexHolder(const MutexHolder &copy);
  INLINE void operator = (const MutexHolder &copy);

private:
#ifdef HAVE_THREADS
  const Mutex *_mutex;
#endif
};

#include "mutexHolder.I"

#endif
