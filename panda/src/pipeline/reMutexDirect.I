// Filename: reMutexDirect.I
// Created by:  drose (13Feb06)
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


////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ReMutexDirect::
ReMutexDirect() {
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::Destructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ReMutexDirect::
~ReMutexDirect() {
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::Copy Constructor
//       Access: Private
//  Description: Do not attempt to copy reMutexes.
////////////////////////////////////////////////////////////////////
INLINE ReMutexDirect::
ReMutexDirect(const ReMutexDirect &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::Copy Assignment Operator
//       Access: Private
//  Description: Do not attempt to copy reMutexes.
////////////////////////////////////////////////////////////////////
INLINE void ReMutexDirect::
operator = (const ReMutexDirect &copy) {
  nassertv(false);
}

#ifdef HAVE_REMUTEXIMPL
////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::lock
//       Access: Public
//  Description: Grabs the reMutex if it is available.  If it is not
//               available, blocks until it becomes available, then
//               grabs it.  In either case, the function does not
//               return until the reMutex is held; you should then call
//               unlock().
//
//               This method is considered const so that you can lock
//               and unlock const reMutexes, mainly to allow thread-safe
//               access to otherwise const data.
//
//               Also see ReMutexHolder.
////////////////////////////////////////////////////////////////////
INLINE void ReMutexDirect::
lock() const {
  ((ReMutexDirect *)this)->_impl.lock();
}
#endif  // HAVE_REMUTEXIMPL

#ifdef HAVE_REMUTEXIMPL
////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::release
//       Access: Public
//  Description: Releases the reMutex.  It is an error to call this if
//               the reMutex was not already locked.
//
//               This method is considered const so that you can lock
//               and unlock const reMutexes, mainly to allow thread-safe
//               access to otherwise const data.
////////////////////////////////////////////////////////////////////
INLINE void ReMutexDirect::
release() const {
  ((ReMutexDirect *)this)->_impl.release();
}
#endif  // HAVE_REMUTEXIMPL

////////////////////////////////////////////////////////////////////
//     Function: ReMutexDirect::debug_is_locked
//       Access: Public
//  Description: Returns true if the current thread has locked the
//               ReMutex, false otherwise.  This method is only intended
//               for use in debugging, hence the method name; in the
//               ReMutexDirect case, it always returns true, since
//               there's not a reliable way to determine this
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool ReMutexDirect::
debug_is_locked() const {
  return true;
}
