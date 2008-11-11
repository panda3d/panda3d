// Filename: virtualFileList.I
// Created by:  drose (03Aug02)
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

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileList::operator []
//       Access: Published
//  Description: Returns the nth file in the list.
////////////////////////////////////////////////////////////////////
INLINE VirtualFile *VirtualFileList::
operator [](int n) const {
  nassertr(n >= 0 && n < (int)_files.size(), NULL);
  return _files[n];
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileList::size
//       Access: Published
//  Description: Returns the number of files in the list.
////////////////////////////////////////////////////////////////////
INLINE int VirtualFileList::
size() const {
  return _files.size();
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileList::operator +=
//       Access: Published
//  Description: Appends the other list onto the end of this one.
////////////////////////////////////////////////////////////////////
INLINE void VirtualFileList::
operator += (const VirtualFileList &other) {
  _files.insert(_files.end(), other._files.begin(), other._files.end());
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileList::operator +
//       Access: Published
//  Description: Returns a VirtualFileList representing the
//               concatenation of the two lists.
////////////////////////////////////////////////////////////////////
INLINE VirtualFileList VirtualFileList::
operator + (const VirtualFileList &other) const {
  VirtualFileList a(*this);
  a += other;
  return a;
}
