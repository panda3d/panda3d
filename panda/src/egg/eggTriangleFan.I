// Filename: eggTriangleFan.I
// Created by:  drose (23Mar05)
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
//     Function: EggTriangleFan::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggTriangleFan::
EggTriangleFan(const string &name) : EggCompositePrimitive(name) {
}

////////////////////////////////////////////////////////////////////
//     Function: EggTriangleFan::Copy constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggTriangleFan::
EggTriangleFan(const EggTriangleFan &copy) : EggCompositePrimitive(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: EggTriangleFan::Copy assignment operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggTriangleFan &EggTriangleFan::
operator = (const EggTriangleFan &copy) {
  EggCompositePrimitive::operator = (copy);
  return *this;
}
