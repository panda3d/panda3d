// Filename: virtualFileSimple.I
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
//     Function: VirtualFileSimple::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE VirtualFileSimple::
VirtualFileSimple(VirtualFileMount *mount, const Filename &local_filename,
                  bool implicit_pz_file) :
  _mount(mount),
  _local_filename(local_filename),
  _implicit_pz_file(implicit_pz_file)
{
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileSimple::is_implicit_pz_file
//       Access: Published
//  Description: Returns true if this file is a .pz file that should
//               be implicitly decompressed on load, or false if it is
//               not a .pz file or if it should not be decompressed.
////////////////////////////////////////////////////////////////////
INLINE bool VirtualFileSimple::
is_implicit_pz_file() const {
  return _implicit_pz_file;
}
