// Filename: mutex.I
// Created by:  drose (08Aug02)
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


////////////////////////////////////////////////////////////////////
//     Function: Mutex::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Mutex::
Mutex() {
}

////////////////////////////////////////////////////////////////////
//     Function: Mutex::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Mutex::
~Mutex() {
}

////////////////////////////////////////////////////////////////////
//     Function: Mutex::Copy Constructor
//       Access: Private
//  Description: Do not attempt to copy mutexes.
////////////////////////////////////////////////////////////////////
INLINE Mutex::
Mutex(const Mutex &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: Mutex::Copy Assignment Operator
//       Access: Private
//  Description: Do not attempt to copy mutexes.
////////////////////////////////////////////////////////////////////
INLINE void Mutex::
operator = (const Mutex &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: Mutex::lock
//       Access: Public
//  Description: Grabs the mutex if it is available.  If it is not
//               available, blocks until it becomes available, then
//               grabs it.  In either case, the function does not
//               return until the mutex is held; you should then call
//               unlock().
//
//               Also see MutexHolder.
////////////////////////////////////////////////////////////////////
INLINE void Mutex::
lock() {
  _impl.lock();
}

////////////////////////////////////////////////////////////////////
//     Function: Mutex::release
//       Access: Public
//  Description: Releases the mutex.  It is an error to call this if
//               the mutex was not already locked.
////////////////////////////////////////////////////////////////////
INLINE void Mutex::
release() {
  _impl.release();
}
