// Filename: withOutputFile.I
// Created by:  drose (01Oct03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: WithOutputFile::set_binary_output
//       Access: Protected
//  Description: Changes the flag specifying whether the output file
//               is to be opened in binary mode or not.
////////////////////////////////////////////////////////////////////
INLINE void WithOutputFile::
set_binary_output(bool binary_output) {
  _binary_output = binary_output;
}
