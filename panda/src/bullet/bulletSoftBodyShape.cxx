// Filename: bulletSoftBodyShape.cxx
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

#include "bulletSoftBodyShape.h"
#include "bulletSoftBodyNode.h"

TypeHandle BulletSoftBodyShape::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyShape::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BulletSoftBodyShape::
BulletSoftBodyShape(btSoftBodyCollisionShape *shapePtr) {

  _shape = shapePtr;
  _shape->setUserPointer(this);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyShape::ptr
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
btCollisionShape *BulletSoftBodyShape::
ptr() const {

  return _shape;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyShape::get_body
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletSoftBodyNode *BulletSoftBodyShape::
get_body() const {

  if (_shape->m_body) {
    return (BulletSoftBodyNode *)_shape->m_body->getUserPointer();
  }
  else
  {
    return NULL;
  }
}

