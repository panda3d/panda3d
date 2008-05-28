// Filename: conditionVarFullDebug.I
// Created by:  drose (28Aug06)
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
//     Function: ConditionVarFullDebug::Copy Constructor
//       Access: Private
//  Description: Do not attempt to copy condition variables.
////////////////////////////////////////////////////////////////////
INLINE ConditionVarFullDebug::
ConditionVarFullDebug(const ConditionVarFullDebug &copy) : 
  _mutex(copy._mutex), 
  _impl(*_mutex._global_lock)
{
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarFullDebug::Copy Assignment Operator
//       Access: Private
//  Description: Do not attempt to copy condition variables.
////////////////////////////////////////////////////////////////////
INLINE void ConditionVarFullDebug::
operator = (const ConditionVarFullDebug &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarFullDebug::get_mutex
//       Access: Published
//  Description: Returns the mutex associated with this condition
//               variable.
////////////////////////////////////////////////////////////////////
INLINE MutexDebug &ConditionVarFullDebug::
get_mutex() const {
  return _mutex;
}
