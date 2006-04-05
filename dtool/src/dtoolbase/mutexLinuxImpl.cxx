// Filename: mutexLinuxImpl.cxx
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

#include "mutexLinuxImpl.h"
#include "atomicAdjust.h"

#include <linux/futex.h>
#include <sys/time.h>
#include <sys/syscall.h>

////////////////////////////////////////////////////////////////////
//     Function: MutexLinuxImpl::lock
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void MutexLinuxImpl::
lock() {
  PN_int32 c;
  c = AtomicAdjust::compare_and_exchange
    (_mode, M_unlocked, M_locked_no_waiters);
  while (c != M_unlocked) {
    if (c == M_locked_with_waiters ||
        AtomicAdjust::compare_and_exchange(_mode, M_locked_no_waiters, 
                                                    M_locked_with_waiters) != M_unlocked) {
      syscall(SYS_futex, &_mode, FUTEX_WAIT, M_locked_with_waiters, (void *)NULL);
    }
    c = AtomicAdjust::compare_and_exchange
      (_mode, M_unlocked, M_locked_with_waiters);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MutexLinuxImpl::release
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void MutexLinuxImpl::
release() {
  if (AtomicAdjust::dec(_mode)) {
    _mode = M_unlocked;
    syscall(SYS_futex, &_mode, FUTEX_WAKE, 1);
  }
}

#endif  // THREAD_LINUX_IMPL
