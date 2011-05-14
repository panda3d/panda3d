// Filename: bulletSoftBodyShape.I
// Created by:  enn0x (06May10)
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
//     Function: BulletSoftBodyShape::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletSoftBodyShape::
~BulletSoftBodyShape() {

  // The btSoftBodyShape _shape is created and deleted internally
  // by the owning btSoftBody object. It must not be deleted
  // here!
}

