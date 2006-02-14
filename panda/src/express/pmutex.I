// Filename: pmutex.I
// Created by:  drose (08Aug02)
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
//     Function: Mutex::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Mutex::
#ifdef DEBUG_THREADS
Mutex() : MutexDebug(false)
#else
Mutex()
#endif  // DEBUG_THREADS
{
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
#ifdef DEBUG_THREADS
Mutex(const Mutex &copy) : MutexDebug(false)
#else
  Mutex(const Mutex &copy)
#endif  // DEBUG_THREADS
{
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
