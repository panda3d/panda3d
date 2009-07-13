// Filename: psemaphore.cxx
// Created by:  drose (13Oct08)
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

#include "psemaphore.h"

////////////////////////////////////////////////////////////////////
//     Function: Semaphore::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void Semaphore::
output(ostream &out) const {
  MutexHolder holder(_lock);
  out << "Semaphore, count = " << _count;
}
