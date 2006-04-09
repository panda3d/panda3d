// Filename: reMutex.I
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
//     Function: ReMutex::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ReMutex::
#ifdef DEBUG_THREADS
ReMutex() : MutexDebug(string(), true)
#else
ReMutex()
#endif  // DEBUG_THREADS
{
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutex::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ReMutex::
#ifdef DEBUG_THREADS
ReMutex(const char *name) : MutexDebug(string(name), true)
#else
ReMutex(const char *)
#endif  // DEBUG_THREADS
{
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutex::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ReMutex::
#ifdef DEBUG_THREADS
ReMutex(const string &name) : MutexDebug(name, true)
#else
ReMutex(const string &)
#endif  // DEBUG_THREADS
{
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutex::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ReMutex::
~ReMutex() {
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutex::Copy Assignment Operator
//       Access: Private
//  Description: Do not attempt to copy mutexes.
////////////////////////////////////////////////////////////////////
INLINE void ReMutex::
operator = (const ReMutex &copy) {
  nassertv(false);
}
