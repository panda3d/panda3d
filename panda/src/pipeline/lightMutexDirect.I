// Filename: lightMutexDirect.I
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


////////////////////////////////////////////////////////////////////
//     Function: LightMutexDirect::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LightMutexDirect::
LightMutexDirect() {
}

////////////////////////////////////////////////////////////////////
//     Function: LightMutexDirect::Destructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LightMutexDirect::
~LightMutexDirect() {
}

////////////////////////////////////////////////////////////////////
//     Function: LightMutexDirect::Copy Constructor
//       Access: Private
//  Description: Do not attempt to copy lightMutexes.
////////////////////////////////////////////////////////////////////
INLINE LightMutexDirect::
LightMutexDirect(const LightMutexDirect &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: LightMutexDirect::Copy Assignment Operator
//       Access: Private
//  Description: Do not attempt to copy lightMutexes.
////////////////////////////////////////////////////////////////////
INLINE void LightMutexDirect::
operator = (const LightMutexDirect &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: LightMutexDirect::acquire
//       Access: Published
//  Description: Grabs the lightMutex if it is available.  If it is not
//               available, blocks until it becomes available, then
//               grabs it.  In either case, the function does not
//               return until the lightMutex is held; you should then call
//               unlock().
//
//               This method is considered const so that you can lock
//               and unlock const lightMutexes, mainly to allow thread-safe
//               access to otherwise const data.
//
//               Also see LightMutexHolder.
////////////////////////////////////////////////////////////////////
INLINE void LightMutexDirect::
acquire() const {
  TAU_PROFILE("void LightMutexDirect::acquire()", " ", TAU_USER);
  ((LightMutexDirect *)this)->_impl.acquire();
}

////////////////////////////////////////////////////////////////////
//     Function: LightMutexDirect::release
//       Access: Published
//  Description: Releases the lightMutex.  It is an error to call this if
//               the lightMutex was not already locked.
//
//               This method is considered const so that you can lock
//               and unlock const lightMutexes, mainly to allow thread-safe
//               access to otherwise const data.
////////////////////////////////////////////////////////////////////
INLINE void LightMutexDirect::
release() const {
  TAU_PROFILE("void LightMutexDirect::release()", " ", TAU_USER);
  ((LightMutexDirect *)this)->_impl.release();
}

////////////////////////////////////////////////////////////////////
//     Function: LightMutexDirect::debug_is_locked
//       Access: Published
//  Description: Returns true if the current thread has locked the
//               LightMutex, false otherwise.  This method is only intended
//               for use in debugging, hence the method name; in the
//               LightMutexDirect case, it always returns true, since
//               there's not a reliable way to determine this
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool LightMutexDirect::
debug_is_locked() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LightMutexDirect::set_name
//       Access: Public
//  Description: The lightMutex name is only defined when compiling in
//               DEBUG_THREADS mode.
////////////////////////////////////////////////////////////////////
INLINE void LightMutexDirect::
set_name(const string &) {
}

////////////////////////////////////////////////////////////////////
//     Function: LightMutexDirect::clear_name
//       Access: Public
//  Description: The lightMutex name is only defined when compiling in
//               DEBUG_THREADS mode.
////////////////////////////////////////////////////////////////////
INLINE void LightMutexDirect::
clear_name() {
}

////////////////////////////////////////////////////////////////////
//     Function: LightMutexDirect::has_name
//       Access: Public
//  Description: The lightMutex name is only defined when compiling in
//               DEBUG_THREADS mode.
////////////////////////////////////////////////////////////////////
INLINE bool LightMutexDirect::
has_name() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: LightMutexDirect::get_name
//       Access: Public
//  Description: The lightMutex name is only defined when compiling in
//               DEBUG_THREADS mode.
////////////////////////////////////////////////////////////////////
INLINE string LightMutexDirect::
get_name() const {
  return string();
}
