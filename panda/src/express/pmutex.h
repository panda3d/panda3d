// Filename: pmutex.h
// Created by:  cary (16Sep98)
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

#ifndef PMUTEX_H
#define PMUTEX_H

#include "pandabase.h"
#include "mutexImpl.h"
#include "notify.h"

////////////////////////////////////////////////////////////////////
//       Class : Mutex
// Description : A standard mutex, or mutual exclusion lock.  Only one
//               thread can hold ("lock") a mutex at any given time;
//               other threads trying to grab the mutex will block
//               until the holding thread releases it.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS Mutex {
public:
  INLINE Mutex();
  INLINE ~Mutex();
private:
  INLINE Mutex(const Mutex &copy);
  INLINE void operator = (const Mutex &copy);

public:
  INLINE void lock() const;
  INLINE void release() const;

private:
  MutexImpl _impl;
  friend class ConditionVar;
};

#include "pmutex.I"

#endif
