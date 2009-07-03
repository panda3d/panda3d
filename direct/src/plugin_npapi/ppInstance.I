// Filename: ppInstance.I
// Created by:  drose (19Jun09)
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
//     Function: PPInstance::get_npp_instance
//       Access: Public
//  Description: Returns the NPP object corresponding to this
//               particular instance.
////////////////////////////////////////////////////////////////////
inline NPP PPInstance::
get_npp_instance() const {
  return _npp_instance;
}
