// Filename: conditionVarLinuxImpl.h
// Created by:  drose (28Mar06)
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

#ifndef CONDITIONVARLINUXIMPL_H
#define CONDITIONVARLINUXIMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#ifdef THREAD_LINUX_IMPL

#include "mutexLinuxImpl.h"
#include "pnotify.h"

class MutexLinuxImpl;

////////////////////////////////////////////////////////////////////
//       Class : ConditionVarLinuxImpl
// Description : Uses Linux threads to implement a conditionVar.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ConditionVarLinuxImpl {
public:
  INLINE ConditionVarLinuxImpl(MutexLinuxImpl &mutex);
  INLINE ~ConditionVarLinuxImpl();

  void wait();
  void signal();

private:
  MutexLinuxImpl &_mutex;
  PN_int32 _counter;
};

#include "conditionVarLinuxImpl.I"

#endif  // THREAD_LINUX_IMPL

#endif
