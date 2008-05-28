// Filename: mutexDirect.cxx
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

#include "mutexDirect.h"

#ifndef DEBUG_THREADS

////////////////////////////////////////////////////////////////////
//     Function: MutexDirect::output
//       Access: Public
//  Description: This method is declared virtual in MutexDebug, but
//               non-virtual in MutexDirect.
////////////////////////////////////////////////////////////////////
void MutexDirect::
output(ostream &out) const {
  out << "Mutex " << (void *)this;
}

#endif  // !DEBUG_THREADS
