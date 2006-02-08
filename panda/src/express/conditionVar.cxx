// Filename: conditionVar.cxx
// Created by:  drose (09Aug02)
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

#include "conditionVar.h"
#include "thread.h"

#ifdef CHECK_REENTRANT_MUTEX
////////////////////////////////////////////////////////////////////
//     Function: ConditionVar::wait
//       Access: Public
//  Description: Waits on the condition.  The caller must already be
//               holding the lock associated with the condition
//               variable before calling this function.
//
//               wait() will release the lock, then go to sleep until
//               some other thread calls signal() on this condition
//               variable.  At that time at least one thread waiting
//               on the same ConditionVar will grab the lock again,
//               and then return from wait().
//
//               It is possible that wait() will return even if no one
//               has called signal().  It is the responsibility of the
//               calling process to verify the condition on return
//               from wait, and possibly loop back to wait again if
//               necessary.
//
//               Note the semantics of a condition variable: the mutex
//               must be held before wait() is called, and it will
//               still be held when wait() returns.  However, it will
//               be temporarily released during the wait() call
//               itself.
//
//               This is defined as an inline method only when
//               CHECK_REENTRANT_MUTEX is not defined.  If
//               CHECK_REENTRANT_MUTEX is defined, it is an
//               out-of-line method, just to avoid circular
//               dependencies on #include files.
////////////////////////////////////////////////////////////////////
void ConditionVar::
wait() {
  nassertv(_mutex.debug_is_locked());
  _impl.wait();

  _mutex._locking_thread = Thread::get_current_thread();
}
#endif  // CHECK_REENTRANT_MUTEX
