// Filename: bulletHeightfieldShape.cxx
// Created by:  enn0x (05Feb10)
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

#include "bulletHeightfieldShape.h"

TypeHandle BulletHeightfieldShape::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BulletHeightfieldShape::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletHeightfieldShape::
BulletHeightfieldShape(const PNMImage &image, PN_stdfloat max_height, BulletUpAxis up) {

  int num_rows = image.get_x_size();
  int num_columns = image.get_y_size();

  _data = new float[num_rows * num_columns];

  for (int row=0; row < num_rows; row++) {
    for (int column=0; column < num_columns; column++) {
      _data[num_columns * row + column] = 
        max_height * image.get_bright(column, num_columns - row - 1);
    }
  }

  _shape = new btHeightfieldTerrainShape(num_rows, num_columns,
                                         _data,
                                         max_height,
                                         up,
                                         true, false);
  _shape->setUserPointer(this);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletHeightfieldShape::ptr
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
btCollisionShape *BulletHeightfieldShape::
ptr() const {

  return _shape;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletHeightfieldShape::set_use_diamond_subdivision
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletHeightfieldShape::
set_use_diamond_subdivision(bool flag) {

  return _shape->setUseDiamondSubdivision(flag);
}

