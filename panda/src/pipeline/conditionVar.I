// Filename: conditionVar.I
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
//     Function: ConditionVar::Constructor
//       Access: Public
//  Description: You must pass in a Mutex to the condition variable
//               constructor.  This mutex may be shared by other
//               condition variables, if desired.  It is the caller's
//               responsibility to ensure the Mutex object does not
//               destruct during the lifetime of the condition
//               variable.
////////////////////////////////////////////////////////////////////
INLINE ConditionVar::
ConditionVar(Mutex &mutex) :
#ifdef DEBUG_THREADS
  ConditionVarDebug(mutex)
#else 
  ConditionVarDirect(mutex)
#endif  // DEBUG_THREADS
{
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVar::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ConditionVar::
~ConditionVar() {
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVar::Copy Constructor
//       Access: Private
//  Description: Do not attempt to copy condition variables.
////////////////////////////////////////////////////////////////////
INLINE ConditionVar::
ConditionVar(const ConditionVar &copy) : 
#ifdef DEBUG_THREADS
  ConditionVarDebug(copy.get_mutex())
#else 
  ConditionVarDirect(copy.get_mutex())
#endif  // DEBUG_THREADS
{
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVar::Copy Assignment Operator
//       Access: Private
//  Description: Do not attempt to copy condition variables.
////////////////////////////////////////////////////////////////////
INLINE void ConditionVar::
operator = (const ConditionVar &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVar::signal_all
//       Access: Private
//  Description: The signal_all() method is specifically *not*
//               provided by ConditionVar.  Use ConditionVarFull if
//               you need to call this method.
////////////////////////////////////////////////////////////////////
INLINE void ConditionVar::
signal_all() {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVar::get_mutex
//       Access: Public
//  Description: Returns the mutex associated with this condition
//               variable.
////////////////////////////////////////////////////////////////////
INLINE Mutex &ConditionVar::
get_mutex() const {
#ifdef DEBUG_THREADS
  return (Mutex &)ConditionVarDebug::get_mutex();
#else 
  return (Mutex &)ConditionVarDirect::get_mutex();
#endif  // DEBUG_THREADS
}
