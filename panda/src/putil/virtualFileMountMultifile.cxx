// Filename: virtualFileMountMultifile.cxx
// Created by:  drose (03Aug02)
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

#include "virtualFileMountMultifile.h"
#include "virtualFileSystem.h"

TypeHandle VirtualFileMountMultifile::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountMultifile::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
VirtualFileMountMultifile::
~VirtualFileMountMultifile() {
  if ((_mount_flags & VirtualFileSystem::MF_owns_pointer) != 0) {
    // Delete the _multifile pointer if we own it.
    nassertv(_multifile != (Multifile *)NULL);
    delete _multifile;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountMultifile::has_file
//       Access: Public, Virtual
//  Description: Returns true if the indicated file exists within the
//               mount system.
////////////////////////////////////////////////////////////////////
bool VirtualFileMountMultifile::
has_file(const Filename &file) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountMultifile::is_directory
//       Access: Public, Virtual
//  Description: Returns true if the indicated file exists within the
//               mount system and is a directory.
////////////////////////////////////////////////////////////////////
bool VirtualFileMountMultifile::
is_directory(const Filename &file) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountMultifile::is_regular_file
//       Access: Public, Virtual
//  Description: Returns true if the indicated file exists within the
//               mount system and is a regular file.
////////////////////////////////////////////////////////////////////
bool VirtualFileMountMultifile::
is_regular_file(const Filename &file) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountMultifile::open_read_file
//       Access: Public, Virtual
//  Description: Opens the file for reading, if it exists.  Returns a
//               newly allocated istream on success (which you should
//               eventually delete when you are done reading).
//               Returns NULL or an invalid istream on failure.
////////////////////////////////////////////////////////////////////
istream *VirtualFileMountMultifile::
open_read_file(const Filename &file) const {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountMultifile::scan_directory
//       Access: Public, Virtual
//  Description: Fills the given vector up with the sorted list of
//               filenames that are local to this directory, if the
//               filename is a directory.  Returns true if successful,
//               or false if the file is not a directory or cannot be
//               read.
////////////////////////////////////////////////////////////////////
bool VirtualFileMountMultifile::
scan_directory(vector_string &contents, const Filename &dir) const {
  return false;
}

