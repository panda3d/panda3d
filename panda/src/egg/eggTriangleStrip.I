// Filename: eggTriangleStrip.I
// Created by:  drose (13Mar05)
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
//     Function: EggTriangleStrip::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggTriangleStrip::
EggTriangleStrip(const string &name) : EggCompositePrimitive(name) {
}

////////////////////////////////////////////////////////////////////
//     Function: EggTriangleStrip::Copy constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggTriangleStrip::
EggTriangleStrip(const EggTriangleStrip &copy) : EggCompositePrimitive(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: EggTriangleStrip::Copy assignment operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggTriangleStrip &EggTriangleStrip::
operator = (const EggTriangleStrip &copy) {
  EggCompositePrimitive::operator = (copy);
  return *this;
}
