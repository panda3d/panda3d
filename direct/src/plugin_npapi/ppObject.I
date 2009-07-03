// Filename: ppObject.I
// Created by:  drose (03Jul09)
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
//     Function: PPObject::set_p3d_object
//       Access: Public
//  Description: Returns the p3d_object this PPObject maps to.  This
//               may be NULL if the object is not fully initialized.
////////////////////////////////////////////////////////////////////
inline P3D_object *PPObject::
get_p3d_object() const {
  return _p3d_object;
}
