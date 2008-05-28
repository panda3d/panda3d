// Filename: xFileDataNodeReference.I
// Created by:  drose (08Oct04)
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
//     Function: XFileDataNodeReference::get_object
//       Access: Public
//  Description: Returns the actual data object being referenced.
////////////////////////////////////////////////////////////////////
INLINE XFileDataNodeTemplate *XFileDataNodeReference::
get_object() const {
  return _object;
}
