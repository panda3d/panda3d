// Filename: virtualFileMountSystem.I
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
//     Function: VirtualFileMountSystem::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE VirtualFileMountSystem::
VirtualFileMountSystem(VirtualFileSystem *file_system,
                       const Filename &physical_filename,
                       const Filename &mount_point,
                       int mount_flags) :
  VirtualFileMount(file_system, physical_filename, mount_point, mount_flags)
{
}
