/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletMinkowskiSumShape.I
 * @author enn0x
 * @date 2010-01-23
 */

/**
 *
 */
INLINE BulletMinkowskiSumShape::
~BulletMinkowskiSumShape() {

  delete _shape;
}

/**
 *
 */
INLINE BulletMinkowskiSumShape::
BulletMinkowskiSumShape(const BulletMinkowskiSumShape &copy) :
  _shape(copy._shape),
  _shape_a(copy._shape_a),
  _shape_b(copy._shape_b) {
}

/**
 *
 */
INLINE void BulletMinkowskiSumShape::
operator = (const BulletMinkowskiSumShape &copy) {
  _shape = copy._shape;
  _shape_a = copy._shape_a;
  _shape_b = copy._shape_b;
}

/**
 *
 */
INLINE void BulletMinkowskiSumShape::
set_transform_a(const TransformState *ts) {

  nassertv(ts);
  _shape->setTransformA(TransformState_to_btTrans(ts));
}

/**
 *
 */
INLINE void BulletMinkowskiSumShape::
set_transform_b(const TransformState *ts) {

  nassertv(ts);
  _shape->setTransformB(TransformState_to_btTrans(ts));
}

/**
 *
 */
INLINE CPT(TransformState) BulletMinkowskiSumShape::
get_transform_a() const {

  return btTrans_to_TransformState(_shape->getTransformA());
}

/**
 *
 */
INLINE CPT(TransformState) BulletMinkowskiSumShape::
get_transform_b() const {

  return btTrans_to_TransformState(_shape->GetTransformB());
}

/**
 *
 */
INLINE const BulletShape *BulletMinkowskiSumShape::
get_shape_a() const {

  return _shape_a;
}

/**
 *
 */
INLINE const BulletShape *BulletMinkowskiSumShape::
get_shape_b() const {

  return _shape_b;
}

/**
 *
 */
INLINE PN_stdfloat BulletMinkowskiSumShape::
get_margin() const {

  return (PN_stdfloat)_shape->getMargin();
}
