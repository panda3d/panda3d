/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightReMutex.I
 * @author drose
 * @date 2008-10-08
 */

////////////////////////////////////////////////////////////////////
//     Function: LightReMutex::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LightReMutex::
#ifdef DEBUG_THREADS
LightReMutex() : MutexDebug(string(), true, true)
#else
LightReMutex()
#endif  // DEBUG_THREADS
{
}

////////////////////////////////////////////////////////////////////
//     Function: LightReMutex::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LightReMutex::
#ifdef DEBUG_THREADS
LightReMutex(const char *name) : MutexDebug(string(name), true, true)
#else
LightReMutex(const char *)
#endif  // DEBUG_THREADS
{
}

////////////////////////////////////////////////////////////////////
//     Function: LightReMutex::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LightReMutex::
#ifdef DEBUG_THREADS
LightReMutex(const string &name) : MutexDebug(name, true, true)
#else
LightReMutex(const string &)
#endif  // DEBUG_THREADS
{
}

////////////////////////////////////////////////////////////////////
//     Function: LightReMutex::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LightReMutex::
~LightReMutex() {
}

////////////////////////////////////////////////////////////////////
//     Function: LightReMutex::Copy Assignment Operator
//       Access: Private
//  Description: Do not attempt to copy mutexes.
////////////////////////////////////////////////////////////////////
INLINE void LightReMutex::
operator = (const LightReMutex &copy) {
  nassertv(false);
}
