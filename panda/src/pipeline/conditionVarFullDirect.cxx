// Filename: conditionVarFullDirect.cxx
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

#include "conditionVarFullDirect.h"

#ifndef DEBUG_THREADS

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarFullDirect::output
//       Access: Public
//  Description: This method is declared virtual in ConditionVarFullDebug,
//               but non-virtual in ConditionVarFullDirect.
////////////////////////////////////////////////////////////////////
void ConditionVarFullDirect::
output(ostream &out) const {
  out << "ConditionVarFull " << (void *)this << " on " << _mutex;
}

#endif  // !DEBUG_THREADS
