// Filename: pmutex.I
// Created by:  drose (08Aug02)
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
//     Function: Mutex::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Mutex::
#ifdef DEBUG_THREADS
Mutex() : MutexDebug(string(), false, false)
#else
Mutex()
#endif  // DEBUG_THREADS
{
}

////////////////////////////////////////////////////////////////////
//     Function: Mutex::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Mutex::
#ifdef DEBUG_THREADS
Mutex(const char *name) : MutexDebug(string(name), false, false)
#else
Mutex(const char *)
#endif  // DEBUG_THREADS
{
}

////////////////////////////////////////////////////////////////////
//     Function: Mutex::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Mutex::
#ifdef DEBUG_THREADS
Mutex(const string &name) : MutexDebug(name, false, false)
#else
Mutex(const string &)
#endif  // DEBUG_THREADS
{
}

////////////////////////////////////////////////////////////////////
//     Function: Mutex::Destructor
//       Access: Published
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
Mutex(const Mutex &copy) : MutexDebug(string(), false, false)
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
