// Filename: reMutexHolder.h
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

#ifndef REMUTEXHOLDER_H
#define REMUTEXHOLDER_H

#include "pandabase.h"
#include "reMutex.h"

////////////////////////////////////////////////////////////////////
//       Class : ReMutexHolder
// Description : Similar to MutexHolder, but for a reentrant mutex.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS ReMutexHolder {
public:
  INLINE ReMutexHolder(const ReMutex &mutex);
  INLINE ReMutexHolder(ReMutex *&mutex);
  INLINE ~ReMutexHolder();
private:
  INLINE ReMutexHolder(const ReMutexHolder &copy);
  INLINE void operator = (const ReMutexHolder &copy);

private:
#if defined(HAVE_THREADS) || !defined(NDEBUG)
  const ReMutex *_mutex;
#endif
};

#include "reMutexHolder.I"

#endif
