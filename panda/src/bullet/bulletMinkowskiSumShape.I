// Filename: bulletMinkowskiSumShape.I
// Created by:  enn0x (23Jan10)
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
//     Function: BulletMinkowskiSumShape::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletMinkowskiSumShape::
~BulletMinkowskiSumShape() {

  delete _shape;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletMinkowskiSumShape::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletMinkowskiSumShape::
BulletMinkowskiSumShape(const BulletMinkowskiSumShape &copy) :
  _shape(copy._shape),
  _shape_a(copy._shape_a),
  _shape_b(copy._shape_b) {
}

////////////////////////////////////////////////////////////////////
//     Function: BulletMinkowskiSumShape::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletMinkowskiSumShape::
operator = (const BulletMinkowskiSumShape &copy) {
  _shape = copy._shape;
  _shape_a = copy._shape_a;
  _shape_b = copy._shape_b;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletMinkowskiSumShape::set_transform_a
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletMinkowskiSumShape::
set_transform_a(const TransformState *ts) {

  nassertv(ts);
  _shape->setTransformA(TransformState_to_btTrans(ts));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletMinkowskiSumShape::set_transform_b
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletMinkowskiSumShape::
set_transform_b(const TransformState *ts) {

  nassertv(ts);
  _shape->setTransformB(TransformState_to_btTrans(ts));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletMinkowskiSumShape::get_transform_a
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) BulletMinkowskiSumShape::
get_transform_a() const {

  return btTrans_to_TransformState(_shape->getTransformA());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletMinkowskiSumShape::get_transform_b
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) BulletMinkowskiSumShape::
get_transform_b() const {

  return btTrans_to_TransformState(_shape->GetTransformB());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletMinkowskiSumShape::get_shape_a
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const BulletShape *BulletMinkowskiSumShape::
get_shape_a() const {

  return _shape_a;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletMinkowskiSumShape::get_shape_b
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const BulletShape *BulletMinkowskiSumShape::
get_shape_b() const {

  return _shape_b;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletMinkowskiSumShape::get_margin
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat BulletMinkowskiSumShape::
get_margin() const {

  return (PN_stdfloat)_shape->getMargin();
}

