// Filename: fileSystemInfo.cxx
// Created by:  drose (20Jun11)
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

#include "fileSystemInfo.h"

////////////////////////////////////////////////////////////////////
//     Function: FileSystemInfo::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void FileSystemInfo::
output(ostream &out) const {
  out << "FileSystemInfo(" << _os_file_name << ", " << _file_start
      << ", " << _file_size << ")";
}
