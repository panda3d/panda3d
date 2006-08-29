// Filename: conditionVarFull.I
// Created by:  drose (28Aug06)
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
//     Function: ConditionVarFull::Constructor
//       Access: Public
//  Description: You must pass in a Mutex to the condition variable
//               constructor.  This mutex may be shared by other
//               condition variables, if desired.  It is the caller's
//               responsibility to ensure the Mutex object does not
//               destruct during the lifetime of the condition
//               variable.
////////////////////////////////////////////////////////////////////
INLINE ConditionVarFull::
ConditionVarFull(Mutex &mutex) :
#ifdef DEBUG_THREADS
  ConditionVarFullDebug(mutex)
#else 
  ConditionVarFullDirect(mutex)
#endif  // DEBUG_THREADS
{
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarFull::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ConditionVarFull::
~ConditionVarFull() {
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarFull::Copy Constructor
//       Access: Private
//  Description: Do not attempt to copy condition variables.
////////////////////////////////////////////////////////////////////
INLINE ConditionVarFull::
ConditionVarFull(const ConditionVarFull &copy) : 
#ifdef DEBUG_THREADS
  ConditionVarFullDebug(copy.get_mutex())
#else 
  ConditionVarFullDirect(copy.get_mutex())
#endif  // DEBUG_THREADS
{
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarFull::Copy Assignment Operator
//       Access: Private
//  Description: Do not attempt to copy condition variables.
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarFull::
operator = (const ConditionVarFull &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarFull::get_mutex
//       Access: Public
//  Description: Returns the mutex associated with this condition
//               variable.
////////////////////////////////////////////////////////////////////
INLINE Mutex &ConditionVarFull::
get_mutex() const {
#ifdef DEBUG_THREADS
  return (Mutex &)ConditionVarFullDebug::get_mutex();
#else 
  return (Mutex &)ConditionVarFullDirect::get_mutex();
#endif  // DEBUG_THREADS
}
