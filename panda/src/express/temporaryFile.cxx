// Filename: temporaryFile.cxx
// Created by:  drose (23Jun11)
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

#include "temporaryFile.h"

TypeHandle TemporaryFile::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TemporaryFile::Destructor
//       Access: Published, Virtual
//  Description: The destructor is responsible for removing the file
//               if it exists.
////////////////////////////////////////////////////////////////////
TemporaryFile::
~TemporaryFile() {
  _filename.unlink();
}
