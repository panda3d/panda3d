// Filename: lightReMutexHolder.h
// Created by:  drose (08Oct08)
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

#ifndef LIGHTREMUTEXHOLDER_H
#define LIGHTREMUTEXHOLDER_H

#include "pandabase.h"
#include "lightReMutex.h"

class Thread;

////////////////////////////////////////////////////////////////////
//       Class : LightReMutexHolder
// Description : Similar to MutexHolder, but for a light reentrant mutex.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PIPELINE LightReMutexHolder {
public:
  INLINE LightReMutexHolder(const LightReMutex &mutex);
  INLINE LightReMutexHolder(const LightReMutex &mutex, Thread *current_thread);
  INLINE LightReMutexHolder(LightReMutex *&mutex);
  INLINE ~LightReMutexHolder();
private:
  INLINE LightReMutexHolder(const LightReMutexHolder &copy);
  INLINE void operator = (const LightReMutexHolder &copy);

private:
#if defined(HAVE_THREADS) || defined(DEBUG_THREADS)
  const LightReMutex *_mutex;
#endif
};

#include "lightReMutexHolder.I"

#endif
