// Filename: checksumHashGenerator.cxx
// Created by:  drose (14May01)
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

#include "checksumHashGenerator.h"

////////////////////////////////////////////////////////////////////
//     Function: ChecksumHashGenerator::add_string
//       Access: Public
//  Description: Adds a string to the hash, by breaking it down into a
//               sequence of integers.
////////////////////////////////////////////////////////////////////
void ChecksumHashGenerator::
add_string(const string &str) {
  add_int(str.length());
  string::const_iterator si;
  for (si = str.begin(); si != str.end(); ++si) {
    add_int(*si);
  }
}
