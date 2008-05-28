// Filename: conditionVarDirect.cxx
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

#include "conditionVarDirect.h"

#ifndef DEBUG_THREADS

////////////////////////////////////////////////////////////////////
//     Function: ConditionVarDirect::output
//       Access: Public
//  Description: This method is declared virtual in ConditionVarDebug,
//               but non-virtual in ConditionVarDirect.
////////////////////////////////////////////////////////////////////
void ConditionVarDirect::
output(ostream &out) const {
  out << "ConditionVar " << (void *)this << " on " << _mutex;
}

#endif  // !DEBUG_THREADS
