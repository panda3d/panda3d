// Filename: virtualFileMountSystem.cxx
// Created by:  drose (03Aug02)
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

#include "virtualFileMountSystem.h"

TypeHandle VirtualFileMountSystem::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountSystem::has_file
//       Access: Public, Virtual
//  Description: Returns true if the indicated file exists within the
//               mount system.
////////////////////////////////////////////////////////////////////
bool VirtualFileMountSystem::
has_file(const Filename &file) const {
  Filename pathname(_physical_filename, file);
  return pathname.exists();
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountSystem::is_directory
//       Access: Public, Virtual
//  Description: Returns true if the indicated file exists within the
//               mount system and is a directory.
////////////////////////////////////////////////////////////////////
bool VirtualFileMountSystem::
is_directory(const Filename &file) const {
  Filename pathname(_physical_filename, file);
  return pathname.is_directory();
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountSystem::is_regular_file
//       Access: Public, Virtual
//  Description: Returns true if the indicated file exists within the
//               mount system and is a regular file.
////////////////////////////////////////////////////////////////////
bool VirtualFileMountSystem::
is_regular_file(const Filename &file) const {
  Filename pathname(_physical_filename, file);
  return pathname.is_regular_file();
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountSystem::open_read_file
//       Access: Public, Virtual
//  Description: Opens the file for reading, if it exists.  Returns a
//               newly allocated istream on success (which you should
//               eventually delete when you are done reading).
//               Returns NULL on failure.
////////////////////////////////////////////////////////////////////
istream *VirtualFileMountSystem::
open_read_file(const Filename &file) const {
  Filename pathname(_physical_filename, file);
  pathname.set_binary();
  ifstream *stream = new ifstream;
  if (!pathname.open_read(*stream)) {
    // Couldn't open the file for some reason.
    close_read_file(stream);
    return NULL;
  }

  return stream;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountSystem::scan_directory
//       Access: Public, Virtual
//  Description: Fills the given vector up with the list of filenames
//               that are local to this directory, if the filename is
//               a directory.  Returns true if successful, or false if
//               the file is not a directory or cannot be read.
////////////////////////////////////////////////////////////////////
bool VirtualFileMountSystem::
scan_directory(vector_string &contents, const Filename &dir) const {
  Filename pathname(_physical_filename, dir);
  return pathname.scan_directory(contents);
}

