// Filename: reMutexHolder.I
// Created by:  drose (15Jan06)
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
//     Function: ReMutexHolder::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ReMutexHolder::
ReMutexHolder(const ReMutex &mutex) {
#if defined(HAVE_THREADS) || !defined(NDEBUG)
  _mutex = &mutex;
  _mutex->lock();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexHolder::Constructor
//       Access: Public
//  Description: This variant on the constructor accepts the current
//               thread as a parameter, if it is already known, as an
//               optimization.
////////////////////////////////////////////////////////////////////
INLINE ReMutexHolder::
ReMutexHolder(const ReMutex &mutex, Thread *current_thread) {
#if defined(HAVE_THREADS) || !defined(NDEBUG)
  _mutex = &mutex;
  _mutex->lock(current_thread);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexHolder::Constructor
//       Access: Public
//  Description: If the ReMutexHolder constructor is given a pointer to
//               a ReMutex object (instead of an actual object), it will
//               first check to see if the pointer is NULL, and
//               allocate a new ReMutex if it is.  This is intended as a
//               convenience for functions that may need to reference
//               a ReMutex at static init time, when it is impossible to
//               guarantee ordering of initializers.
////////////////////////////////////////////////////////////////////
INLINE ReMutexHolder::
ReMutexHolder(ReMutex *&mutex) {
#if defined(HAVE_THREADS) || !defined(NDEBUG)
  if (mutex == (ReMutex *)NULL) {
    mutex = new ReMutex;
  }
  _mutex = mutex;
  _mutex->lock();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexHolder::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ReMutexHolder::
~ReMutexHolder() {
#if defined(HAVE_THREADS) || !defined(NDEBUG)
  _mutex->release();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexHolder::Copy Constructor
//       Access: Private
//  Description: Do not attempt to copy ReMutexHolders.
////////////////////////////////////////////////////////////////////
INLINE ReMutexHolder::
ReMutexHolder(const ReMutexHolder &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexHolder::Copy Assignment Operator
//       Access: Private
//  Description: Do not attempt to copy ReMutexHolders.
////////////////////////////////////////////////////////////////////
INLINE void ReMutexHolder::
operator = (const ReMutexHolder &copy) {
  nassertv(false);
}
