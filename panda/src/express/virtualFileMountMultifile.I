// Filename: virtualFileMountMultifile.I
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
//     Function: VirtualFileMountMultifile::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE VirtualFileMountMultifile::
VirtualFileMountMultifile(VirtualFileSystem *file_system,
                          Multifile *multifile,
                          const Filename &mount_point,
                          int mount_flags) :
  VirtualFileMount(file_system, multifile->get_multifile_name(),
                   mount_point, mount_flags),
  _multifile(multifile)
{
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMountMultifile::get_multifile
//       Access: Public
//  Description: Returns the Multifile pointer that this mount object
//               is based on.
////////////////////////////////////////////////////////////////////
INLINE Multifile *VirtualFileMountMultifile::
get_multifile() const {
  return _multifile;
}
