// Filename: mutexDirect.I
// Created by:  drose (13Feb06)
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
//     Function: MutexDirect::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MutexDirect::
MutexDirect() {
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDirect::Destructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MutexDirect::
~MutexDirect() {
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDirect::Copy Constructor
//       Access: Private
//  Description: Do not attempt to copy mutexes.
////////////////////////////////////////////////////////////////////
INLINE MutexDirect::
MutexDirect(const MutexDirect &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDirect::Copy Assignment Operator
//       Access: Private
//  Description: Do not attempt to copy mutexes.
////////////////////////////////////////////////////////////////////
INLINE void MutexDirect::
operator = (const MutexDirect &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDirect::acquire
//       Access: Published
//  Description: Grabs the mutex if it is available.  If it is not
//               available, blocks until it becomes available, then
//               grabs it.  In either case, the function does not
//               return until the mutex is held; you should then call
//               unlock().
//
//               This method is considered const so that you can lock
//               and unlock const mutexes, mainly to allow thread-safe
//               access to otherwise const data.
//
//               Also see MutexHolder.
////////////////////////////////////////////////////////////////////
INLINE void MutexDirect::
acquire() const {
  TAU_PROFILE("void MutexDirect::acquire()", " ", TAU_USER);
  ((MutexDirect *)this)->_impl.acquire();
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDirect::try_acquire
//       Access: Published
//  Description: Returns immediately, with a true value indicating the
//               mutex has been acquired, and false indicating it has
//               not.
////////////////////////////////////////////////////////////////////
INLINE bool MutexDirect::
try_acquire() const {
  TAU_PROFILE("void MutexDirect::acquire(bool)", " ", TAU_USER);
  return ((MutexDirect *)this)->_impl.try_acquire();
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDirect::release
//       Access: Published
//  Description: Releases the mutex.  It is an error to call this if
//               the mutex was not already locked.
//
//               This method is considered const so that you can lock
//               and unlock const mutexes, mainly to allow thread-safe
//               access to otherwise const data.
////////////////////////////////////////////////////////////////////
INLINE void MutexDirect::
release() const {
  TAU_PROFILE("void MutexDirect::release()", " ", TAU_USER);
  ((MutexDirect *)this)->_impl.release();
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDirect::debug_is_locked
//       Access: Published
//  Description: Returns true if the current thread has locked the
//               Mutex, false otherwise.  This method is only intended
//               for use in debugging, hence the method name; in the
//               MutexDirect case, it always returns true, since
//               there's not a reliable way to determine this
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool MutexDirect::
debug_is_locked() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDirect::set_name
//       Access: Public
//  Description: The mutex name is only defined when compiling in
//               DEBUG_THREADS mode.
////////////////////////////////////////////////////////////////////
INLINE void MutexDirect::
set_name(const string &) {
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDirect::clear_name
//       Access: Public
//  Description: The mutex name is only defined when compiling in
//               DEBUG_THREADS mode.
////////////////////////////////////////////////////////////////////
INLINE void MutexDirect::
clear_name() {
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDirect::has_name
//       Access: Public
//  Description: The mutex name is only defined when compiling in
//               DEBUG_THREADS mode.
////////////////////////////////////////////////////////////////////
INLINE bool MutexDirect::
has_name() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: MutexDirect::get_name
//       Access: Public
//  Description: The mutex name is only defined when compiling in
//               DEBUG_THREADS mode.
////////////////////////////////////////////////////////////////////
INLINE string MutexDirect::
get_name() const {
  return string();
}
