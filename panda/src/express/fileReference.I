// Filename: fileReference.I
// Created by:  drose (23Jun11)
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
//     Function: FileReference::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FileReference::
FileReference(const Filename &filename) : _filename(filename) {
}
 
////////////////////////////////////////////////////////////////////
//     Function: FileReference::get_filename
//       Access: Published
//  Description: Returns the filename of the reference.
////////////////////////////////////////////////////////////////////
INLINE const Filename &FileReference::
get_filename() const {
  return _filename;
}
