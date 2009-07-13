// Filename: psemaphore.I
// Created by:  drose (13Oct08)
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


////////////////////////////////////////////////////////////////////
//     Function: Semaphore::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE Semaphore::
Semaphore(int initial_count) :
  _lock("Semaphore::_lock"),
  _cvar(_lock),
  _count(initial_count)
{
  nassertv(_count >= 0);
}

////////////////////////////////////////////////////////////////////
//     Function: Semaphore::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE Semaphore::
~Semaphore() {
}

////////////////////////////////////////////////////////////////////
//     Function: Semaphore::Copy Constructor
//       Access: Private
//  Description: Do not attempt to copy semaphores.
////////////////////////////////////////////////////////////////////
INLINE Semaphore::
Semaphore(const Semaphore &copy) : 
  _cvar(_lock)
{
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: Semaphore::Copy Assignment Operator
//       Access: Private
//  Description: Do not attempt to copy semaphores.
////////////////////////////////////////////////////////////////////
INLINE void Semaphore::
operator = (const Semaphore &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: Semaphore::acquire
//       Access: Published
//  Description: Decrements the internal count.  If the count was
//               already at zero, blocks until the count is nonzero,
//               then decrements it.
////////////////////////////////////////////////////////////////////
INLINE void Semaphore::
acquire() {
  TAU_PROFILE("void Semaphore::acquire()", " ", TAU_USER);
  MutexHolder holder(_lock);
  nassertv(_count >= 0);
  while (_count <= 0) {
    _cvar.wait();
  }
  --_count;
}

////////////////////////////////////////////////////////////////////
//     Function: Semaphore::try_acquire
//       Access: Published
//  Description: If the semaphore can be acquired without blocking,
//               does so and returns true.  Otherwise, returns false.
////////////////////////////////////////////////////////////////////
INLINE bool Semaphore::
try_acquire() {
  TAU_PROFILE("void Semaphore::acquire(bool)", " ", TAU_USER);
  MutexHolder holder(_lock);
  nassertr(_count >= 0, false);
  if (_count <= 0) {
    return false;
  }
  --_count;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Semaphore::release
//       Access: Published
//  Description: Increments the semaphore's internal count.  This may
//               wake up another thread blocked on acquire().
//
//               Returns the count of the semaphore upon release.
////////////////////////////////////////////////////////////////////
INLINE int Semaphore::
release() {
  TAU_PROFILE("void Semaphore::release()", " ", TAU_USER);
  MutexHolder holder(_lock);
  ++_count;
  _cvar.notify();
  return _count;
}

////////////////////////////////////////////////////////////////////
//     Function: Semaphore::get_count
//       Access: Published
//  Description: Returns the current semaphore count.  Note that this
//               call is not thread-safe (the count may change at any
//               time).
////////////////////////////////////////////////////////////////////
INLINE int Semaphore::
get_count() const {
  TAU_PROFILE("void Semaphore::get_count()", " ", TAU_USER);
  MutexHolder holder(_lock);
  return _count;
}
