// Filename: mutexHolder.I
// Created by:  drose (09Aug02)
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
//     Function: MutexHolder::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MutexHolder::
MutexHolder(const Mutex &mutex) {
#if defined(HAVE_THREADS) || defined(DEBUG_THREADS)
  _mutex = &mutex;
  _mutex->acquire();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: MutexHolder::Constructor
//       Access: Public
//  Description: This variant on the constructor accepts the current
//               thread as a parameter, if it is already known, as an
//               optimization.
////////////////////////////////////////////////////////////////////
INLINE MutexHolder::
MutexHolder(const Mutex &mutex, Thread *current_thread) {
#if defined(HAVE_THREADS) || defined(DEBUG_THREADS)
  _mutex = &mutex;
  // Actually, the regular Mutex class doesn't need the current thread
  // parameter at the moment.  So not actually an optimization.  But
  // we keep this method because it causes a symmetry with
  // ReMutexHolder.
  _mutex->acquire(/*current_thread*/);
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
#if defined(HAVE_THREADS) || defined(DEBUG_THREADS)
  if (mutex == (Mutex *)NULL) {
    mutex = new Mutex;
  }
  _mutex = mutex;
  _mutex->acquire();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: MutexHolder::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MutexHolder::
~MutexHolder() {
#if defined(HAVE_THREADS) || defined(DEBUG_THREADS)
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
