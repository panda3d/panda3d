// Filename: lightMutexHolder.h
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

#ifndef LIGHTMUTEXHOLDER_H
#define LIGHTMUTEXHOLDER_H

#include "pandabase.h"
#include "lightMutex.h"

class Thread;

////////////////////////////////////////////////////////////////////
//       Class : LightMutexHolder
// Description : Similar to MutexHolder, but for a light mutex.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PIPELINE LightMutexHolder {
public:
  INLINE LightMutexHolder(const LightMutex &mutex);
  INLINE LightMutexHolder(LightMutex *&mutex);
  INLINE ~LightMutexHolder();
private:
  INLINE LightMutexHolder(const LightMutexHolder &copy);
  INLINE void operator = (const LightMutexHolder &copy);

private:
#if defined(HAVE_THREADS) || defined(DEBUG_THREADS)
  const LightMutex *_mutex;
#endif
};

#include "lightMutexHolder.I"

#endif
