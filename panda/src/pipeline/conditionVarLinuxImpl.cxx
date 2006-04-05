// Filename: conditionVarLinuxImpl.cxx
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

#include "selectThreadImpl.h"

#ifdef THREAD_LINUX_IMPL

#include "conditionVarLinuxImpl.h"

#include <linux/futex.h>
#include <sys/time.h>
#include <sys/syscall.h>

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarLinuxImpl::wait
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void ConditionVarLinuxImpl::
wait() {
  // Grab the current value of the counter before we release the
  // mutex.
  PN_int32 orig_counter = _counter;
  _mutex.release();
  syscall(SYS_futex, &_counter, FUTEX_WAIT, orig_counter, (void *)NULL);
  _mutex.lock();
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarLinuxImpl::signal
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void ConditionVarLinuxImpl::
signal() {
  AtomicAdjust::inc(_counter);
  syscall(SYS_futex, &_counter, FUTEX_WAKE, 1);
}

#endif  // THREAD_LINUX_IMPL
