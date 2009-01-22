// Filename: lightReMutexDirect.cxx
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

#include "lightReMutexDirect.h"
#include "thread.h"

#ifndef DEBUG_THREADS

////////////////////////////////////////////////////////////////////
//     Function: LightReMutexDirect::output
//       Access: Published
//  Description: This method is declared virtual in MutexDebug, but
//               non-virtual in LightReMutexDirect.
////////////////////////////////////////////////////////////////////
void LightReMutexDirect::
output(ostream &out) const {
  out << "LightReMutex " << (void *)this;
}

#endif  // !DEBUG_THREADS
