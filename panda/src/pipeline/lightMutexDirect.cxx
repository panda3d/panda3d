// Filename: lightMutexDirect.cxx
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

#include "lightMutexDirect.h"

#ifndef DEBUG_THREADS

////////////////////////////////////////////////////////////////////
//     Function: LightMutexDirect::output
//       Access: Public
//  Description: This method is declared virtual in LightMutexDebug, but
//               non-virtual in LightMutexDirect.
////////////////////////////////////////////////////////////////////
void LightMutexDirect::
output(ostream &out) const {
  out << "LightMutex " << (void *)this;
}

#endif  // !DEBUG_THREADS
