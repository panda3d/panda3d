// Filename: mutexHolder.I
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


////////////////////////////////////////////////////////////////////
//     Function: MutexHolder::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MutexHolder::
MutexHolder(const Mutex &mutex) {
#ifdef HAVE_THREADS
  _mutex = &mutex;
  _mutex->lock();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: MutexHolder::Constructor
//       Access: Public
//  Description: If the MutexHolder constructor is given a pointer to
//               a Mutex object (instead of an actual object), it will
//               first check to see if the pointer is NULL, and
//               allocate a new Mutex if it is.  This is intended as a
//               convenience for functions that may need to reference
//               a Mutex at static init time, when it is impossible to
//               guarantee ordering of initializers.
////////////////////////////////////////////////////////////////////
INLINE MutexHolder::
MutexHolder(Mutex *&mutex) {
#ifdef HAVE_THREADS
  if (mutex == (Mutex *)NULL) {
    mutex = new Mutex;
  }
  _mutex = mutex;
  _mutex->lock();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: MutexHolder::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MutexHolder::
~MutexHolder() {
#ifdef HAVE_THREADS
  _mutex->release();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: MutexHolder::Copy Constructor
//       Access: Private
//  Description: Do not attempt to copy MutexHolders.
////////////////////////////////////////////////////////////////////
INLINE MutexHolder::
MutexHolder(const MutexHolder &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: MutexHolder::Copy Assignment Operator
//       Access: Private
//  Description: Do not attempt to copy MutexHolders.
////////////////////////////////////////////////////////////////////
INLINE void MutexHolder::
operator = (const MutexHolder &copy) {
  nassertv(false);
}
