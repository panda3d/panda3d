// Filename: virtualFileMount.I
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
//     Function: VirtualFileMount::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE VirtualFileMount::
VirtualFileMount() :
  _file_system(NULL),
  _mount_flags(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMount::get_file_system
//       Access: Public
//  Description: Returns the file system this mount object is attached
//               to.
////////////////////////////////////////////////////////////////////
INLINE VirtualFileSystem *VirtualFileMount::
get_file_system() const {
  return _file_system;
}


////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMount::get_mount_point
//       Access: Public
//  Description: Returns the name of the directory within the virtual
//               file system that this mount object is attached to.
//               This directory name will end with a slash.
////////////////////////////////////////////////////////////////////
INLINE const Filename &VirtualFileMount::
get_mount_point() const {
  return _mount_point;
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMount::get_mount_flags
//       Access: Public
//  Description: Returns the set of flags passed by the user to the
//               VirtualFileSystem::mount() command.
////////////////////////////////////////////////////////////////////
INLINE int VirtualFileMount::
get_mount_flags() const {
  return _mount_flags;
}


INLINE ostream &
operator << (ostream &out, const VirtualFileMount &mount) {
  mount.output(out);
  return out;
}
