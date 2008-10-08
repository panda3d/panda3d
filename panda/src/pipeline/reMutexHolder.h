// Filename: reMutexHolder.h
// Created by:  drose (15Jan06)
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

#ifndef REMUTEXHOLDER_H
#define REMUTEXHOLDER_H

#include "pandabase.h"
#include "reMutex.h"

class Thread;

////////////////////////////////////////////////////////////////////
//       Class : ReMutexHolder
// Description : Similar to MutexHolder, but for a reentrant mutex.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PIPELINE ReMutexHolder {
public:
  INLINE ReMutexHolder(const ReMutex &mutex);
  INLINE ReMutexHolder(const ReMutex &mutex, Thread *current_thread);
  INLINE ReMutexHolder(ReMutex *&mutex);
  INLINE ~ReMutexHolder();
private:
  INLINE ReMutexHolder(const ReMutexHolder &copy);
  INLINE void operator = (const ReMutexHolder &copy);

private:
#if defined(HAVE_THREADS) || defined(DEBUG_THREADS)
  const ReMutex *_mutex;
#endif
};

#include "reMutexHolder.I"

#endif
