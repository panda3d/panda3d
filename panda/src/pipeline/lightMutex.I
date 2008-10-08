// Filename: lightMutex.I
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
//     Function: LightMutex::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LightMutex::
#ifdef DEBUG_THREADS
LightMutex() : MutexDebug(string(), false, true)
#else
LightMutex()
#endif  // DEBUG_THREADS
{
}

////////////////////////////////////////////////////////////////////
//     Function: LightMutex::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LightMutex::
#ifdef DEBUG_THREADS
LightMutex(const char *name) : MutexDebug(string(name), false, true)
#else
LightMutex(const char *)
#endif  // DEBUG_THREADS
{
}

////////////////////////////////////////////////////////////////////
//     Function: LightMutex::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LightMutex::
#ifdef DEBUG_THREADS
LightMutex(const string &name) : MutexDebug(name, false, true)
#else
LightMutex(const string &)
#endif  // DEBUG_THREADS
{
}

////////////////////////////////////////////////////////////////////
//     Function: LightMutex::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LightMutex::
~LightMutex() {
}

////////////////////////////////////////////////////////////////////
//     Function: LightMutex::Copy Constructor
//       Access: Private
//  Description: Do not attempt to copy lightMutexes.
////////////////////////////////////////////////////////////////////
INLINE LightMutex::
#ifdef DEBUG_THREADS
LightMutex(const LightMutex &copy) : MutexDebug(string(), false, true)
#else
  LightMutex(const LightMutex &copy)
#endif  // DEBUG_THREADS
{
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: LightMutex::Copy Assignment Operator
//       Access: Private
//  Description: Do not attempt to copy lightMutexes.
////////////////////////////////////////////////////////////////////
INLINE void LightMutex::
operator = (const LightMutex &copy) {
  nassertv(false);
}
