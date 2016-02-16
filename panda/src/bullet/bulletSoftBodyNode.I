/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletSoftBodyNode.I
 * @author enn0x
 * @date 2010-12-27
 */

/**

 */
INLINE BulletSoftBodyNode::
~BulletSoftBodyNode() {

  delete _soft;
}

/**

 */
INLINE BulletSoftBodyNodeElement::
~BulletSoftBodyNodeElement() {

}

/**
 * Named constructor intended to be used for asserts with have to return a
 * concrete value.
 */
INLINE BulletSoftBodyNodeElement BulletSoftBodyNodeElement::
empty() {

  static btSoftBody::Node node;

  return BulletSoftBodyNodeElement(node);
}

/**

 */
INLINE LPoint3 BulletSoftBodyNodeElement::
get_pos() const {

  return btVector3_to_LPoint3(_node.m_x);
}

/**

 */
INLINE LVector3 BulletSoftBodyNodeElement::
get_normal() const {

  return btVector3_to_LVector3(_node.m_n);
}

/**

 */
INLINE LVector3 BulletSoftBodyNodeElement::
get_velocity() const {

  return btVector3_to_LVector3(_node.m_v);
}

/**

 */
INLINE PN_stdfloat BulletSoftBodyNodeElement::
get_inv_mass() const {

  return (PN_stdfloat)_node.m_im;
}

/**

 */
INLINE PN_stdfloat BulletSoftBodyNodeElement::
get_area() const {

  return (PN_stdfloat)_node.m_area;
}

/**

 */
INLINE int BulletSoftBodyNodeElement::
is_attached() const {

  return (PN_stdfloat)_node.m_battach;
}
