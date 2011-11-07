// Filename: virtualFileMountRamdisk.I
// Created by:  drose (19Sep11)
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
//     Function: VirtualFileMountRamdisk::FileBase::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE VirtualFileMountRamdisk::FileBase::
FileBase(const string &basename) : _basename(basename), _timestamp(0) {
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountRamdisk::FileBase::operator <
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool VirtualFileMountRamdisk::FileBase::
operator < (const FileBase &other) const {
  return _basename < other._basename;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountRamdisk::File::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE VirtualFileMountRamdisk::File::
File(const string &basename) : 
  FileBase(basename),
  _wrapper(&_data, false, true)
{
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountRamdisk::Directory::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE VirtualFileMountRamdisk::Directory::
Directory(const string &basename) : FileBase(basename) {
}
