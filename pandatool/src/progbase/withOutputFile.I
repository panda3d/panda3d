// Filename: withOutputFile.I
// Created by:  drose (01Oct03)
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
//     Function: WithOutputFile::set_binary_output
//       Access: Protected
//  Description: Changes the flag specifying whether the output file
//               is to be opened in binary mode or not.
////////////////////////////////////////////////////////////////////
INLINE void WithOutputFile::
set_binary_output(bool binary_output) {
  _binary_output = binary_output;
}
