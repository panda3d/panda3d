// Filename: virtualFileList.I
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


////////////////////////////////////////////////////////////////////
//     Function: VirtualFileList::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE VirtualFileList::
VirtualFileList() {
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileList::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE VirtualFileList::
~VirtualFileList() {
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileList::add_file
//       Access: Public
//  Description: Adds a new file to the list.
////////////////////////////////////////////////////////////////////
INLINE void VirtualFileList::
add_file(VirtualFile *file) {
  _files.push_back(file);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileList::get_num_files
//       Access: Published
//  Description: Returns the number of files in the list.
////////////////////////////////////////////////////////////////////
INLINE int VirtualFileList::
get_num_files() const {
  return _files.size();
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileList::get_file
//       Access: Published
//  Description: Returns the nth file in the list.
////////////////////////////////////////////////////////////////////
INLINE VirtualFile *VirtualFileList::
get_file(int n) const {
  nassertr(n >= 0 && n < (int)_files.size(), NULL);
  return _files[n];
}
