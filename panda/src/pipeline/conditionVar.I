// Filename: conditionVar.I
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
//     Function: ConditionVar::Constructor
//       Access: Published
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
//       Access: Published
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
//     Function: ConditionVar::notify_all
//       Access: Private
//  Description: The notify_all() method is specifically *not*
//               provided by ConditionVar.  Use ConditionVarFull if
//               you need to call this method.
////////////////////////////////////////////////////////////////////
INLINE void ConditionVar::
notify_all() {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVar::get_mutex
//       Access: Published
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
