// Filename: conditionVarDebug.I
// Created by:  drose (13Feb06)
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
//     Function: ConditionVarDebug::Copy Constructor
//       Access: Private
//  Description: Do not attempt to copy condition variables.
////////////////////////////////////////////////////////////////////
INLINE ConditionVarDebug::
ConditionVarDebug(const ConditionVarDebug &copy) : 
  _mutex(copy._mutex), 
  _impl(*_mutex._global_lock)
{
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarDebug::Copy Assignment Operator
//       Access: Private
//  Description: Do not attempt to copy condition variables.
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarDebug::
operator = (const ConditionVarDebug &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarDebug::get_mutex
//       Access: Published
//  Description: Returns the mutex associated with this condition
//               variable.
////////////////////////////////////////////////////////////////////
INLINE MutexDebug &ConditionVarDebug::
get_mutex() const {
  return _mutex;
}
