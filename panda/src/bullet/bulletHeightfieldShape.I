// Filename: bulletHeightfieldShape.I
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

////////////////////////////////////////////////////////////////////
//     Function: BulletHeightfieldShape::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletHeightfieldShape::
~BulletHeightfieldShape() {

  delete _shape;
  delete _data;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletHeightfieldShape::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletHeightfieldShape::
BulletHeightfieldShape(const BulletHeightfieldShape &copy) :
  _shape(copy._shape),
  _num_rows(copy._num_rows),
  _num_cols(copy._num_cols) {

  _data = new float[_num_rows * _num_cols];
  memcpy(_data, copy._data, _num_rows * _num_cols * sizeof(float));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletHeightfieldShape::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletHeightfieldShape::
operator = (const BulletHeightfieldShape &copy) {

  _shape = copy._shape;
  _num_rows = copy._num_rows;
  _num_cols = copy._num_cols;

  _data = new float[_num_rows * _num_cols];
  memcpy(_data, copy._data, _num_rows * _num_cols * sizeof(float));
}

