// Filename: eggTriangleStrip.I
// Created by:  drose (13Mar05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
