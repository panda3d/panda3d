// Filename: p3dCInstance.I
// Created by:  drose (08Jun09)
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
//     Function: P3DCInstance::get_instance_id
//       Access: Public
//  Description: Returns a unique integer for each instance in the
//               system.
////////////////////////////////////////////////////////////////////
inline int P3DCInstance::
get_instance_id() const {
  return _instance_id;
}
