// Filename: checksumHashGenerator.cxx
// Created by:  drose (14May01)
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
