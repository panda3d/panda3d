// Filename: characterJoint.I
// Created by:  drose (20Nov08)
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
//     Function: CharacterJoint::get_transform
//       Access: Published
//  Description: Returns the transform matrix of the joint
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4 &CharacterJoint::
get_transform() const {
  return _value;
}