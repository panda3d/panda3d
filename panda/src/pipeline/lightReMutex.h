// Filename: lightReMutex.h
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

#ifndef LIGHTREMUTEX_H
#define LIGHTREMUTEX_H

#include "pandabase.h"
#include "mutexDebug.h"
#include "lightReMutexDirect.h"

////////////////////////////////////////////////////////////////////
//       Class : LightReMutex
// Description : A lightweight reentrant mutex.  See LightMutex and
//               ReMutex.
//
//               This class inherits its implementation either from
//               MutexDebug or LightReMutexDirect, depending on the
//               definition of DEBUG_THREADS.
////////////////////////////////////////////////////////////////////
#ifdef DEBUG_THREADS
class EXPCL_PANDA_PIPELINE LightReMutex : public MutexDebug
#else
class EXPCL_PANDA_PIPELINE LightReMutex : public LightReMutexDirect
#endif  // DEBUG_THREADS
{
PUBLISHED:
  INLINE LightReMutex();
public:
  INLINE LightReMutex(const char *name);
PUBLISHED:
  INLINE LightReMutex(const string &name);
  INLINE ~LightReMutex();
private:
  INLINE LightReMutex(const LightReMutex &copy);
  INLINE void operator = (const LightReMutex &copy);
};

#include "lightReMutex.I"

#endif
