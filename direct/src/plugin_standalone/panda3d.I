// Filename: panda3d.I
// Created by:  drose (30Jun09)
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
//     Function: Panda3D::URLGetter::get_instance
//       Access: Public
//  Description: Returns the P3D_instance associated with this
//               URLGetter.
////////////////////////////////////////////////////////////////////
P3D_instance *Panda3D::URLGetter::
get_instance() {
  return _instance;
}

