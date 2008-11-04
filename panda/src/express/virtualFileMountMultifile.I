// Filename: virtualFileMountMultifile.I
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
//     Function: VirtualFileMountMultifile::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE VirtualFileMountMultifile::
VirtualFileMountMultifile(Multifile *multifile) :
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
