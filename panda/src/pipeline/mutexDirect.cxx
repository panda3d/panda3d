// Filename: mutexDirect.cxx
// Created by:  drose (13Feb06)
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
