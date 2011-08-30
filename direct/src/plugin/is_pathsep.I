// Filename: is_pathsep.I
// Created by:  drose (07Jul09)
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
//     Function: is_pathsep
//  Description: Returns true if the indicated character is a path
//               separator character (e.g. slash or backslash), false
//               otherwise.
////////////////////////////////////////////////////////////////////
inline bool
is_pathsep(int ch) {
  if (ch == '/') {
    return true;
  }
#ifdef _WIN32
  if (ch == '\\') {
    return true;
  }
#endif
  return false;
}
