// Filename: virtualFileMount.I
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
//     Function: VirtualFileMount::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE VirtualFileMount::
VirtualFileMount(VirtualFileSystem *file_system,
                 const Filename &physical_filename,
                 const Filename &mount_point,
                 int mount_flags) :
  _file_system(file_system),
  _physical_filename(physical_filename),
  _mount_point(mount_point),
  _mount_flags(mount_flags)
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
//     Function: VirtualFileMount::get_physical_filename
//       Access: Public
//  Description: Returns the name of the source file on the OS
//               filesystem of the directory or file that is mounted.
////////////////////////////////////////////////////////////////////
INLINE const Filename &VirtualFileMount::
get_physical_filename() const {
  return _physical_filename;
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
