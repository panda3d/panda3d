// Filename: lightReMutexHolder.I
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
//     Function: LightReMutexHolder::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LightReMutexHolder::
LightReMutexHolder(const LightReMutex &mutex) {
#if defined(HAVE_THREADS) || defined(DEBUG_THREADS)
  _mutex = &mutex;
  _mutex->acquire();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: LightReMutexHolder::Constructor
//       Access: Public
//  Description: This variant on the constructor accepts the current
//               thread as a parameter, if it is already known, as an
//               optimization.
////////////////////////////////////////////////////////////////////
INLINE LightReMutexHolder::
LightReMutexHolder(const LightReMutex &mutex, Thread *current_thread) {
#if defined(HAVE_THREADS) || defined(DEBUG_THREADS)
  _mutex = &mutex;
  _mutex->acquire(current_thread);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: LightReMutexHolder::Constructor
//       Access: Public
//  Description: If the LightReMutexHolder constructor is given a pointer to
//               a LightReMutex object (instead of an actual object), it will
//               first check to see if the pointer is NULL, and
//               allocate a new LightReMutex if it is.  This is intended as a
//               convenience for functions that may need to reference
//               a LightReMutex at static init time, when it is impossible to
//               guarantee ordering of initializers.
////////////////////////////////////////////////////////////////////
INLINE LightReMutexHolder::
LightReMutexHolder(LightReMutex *&mutex) {
#if defined(HAVE_THREADS) || defined(DEBUG_THREADS)
  if (mutex == (LightReMutex *)NULL) {
    mutex = new LightReMutex;
  }
  _mutex = mutex;
  _mutex->acquire();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: LightReMutexHolder::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LightReMutexHolder::
~LightReMutexHolder() {
#if defined(HAVE_THREADS) || defined(DEBUG_THREADS)
  _mutex->release();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: LightReMutexHolder::Copy Constructor
//       Access: Private
//  Description: Do not attempt to copy LightReMutexHolders.
////////////////////////////////////////////////////////////////////
INLINE LightReMutexHolder::
LightReMutexHolder(const LightReMutexHolder &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: LightReMutexHolder::Copy Assignment Operator
//       Access: Private
//  Description: Do not attempt to copy LightReMutexHolders.
////////////////////////////////////////////////////////////////////
INLINE void LightReMutexHolder::
operator = (const LightReMutexHolder &copy) {
  nassertv(false);
}
