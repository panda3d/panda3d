// Filename: bulletSoftBodyNode.I
// Created by:  enn0x (27Dec10)
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
//     Function: BulletSoftBodyNode::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletSoftBodyNode::
~BulletSoftBodyNode() {

  delete _soft;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNodeElement::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletSoftBodyNodeElement::
~BulletSoftBodyNodeElement() {

}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNodeElement::empty
//       Access: Published
//  Description: Named constructor intended to be used for asserts
//               with have to return a concrete value.
////////////////////////////////////////////////////////////////////
INLINE BulletSoftBodyNodeElement BulletSoftBodyNodeElement::
empty() {

  static btSoftBody::Node node;

  return BulletSoftBodyNodeElement(node);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNodeElement::get_pos
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LPoint3 BulletSoftBodyNodeElement::
get_pos() const {

  return btVector3_to_LPoint3(_node.m_x);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNodeElement::get_normal
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LVector3 BulletSoftBodyNodeElement::
get_normal() const {

  return btVector3_to_LVector3(_node.m_n);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNodeElement::get_velocity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LVector3 BulletSoftBodyNodeElement::
get_velocity() const {

  return btVector3_to_LVector3(_node.m_v);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNodeElement::get_inv_mass
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat BulletSoftBodyNodeElement::
get_inv_mass() const {

  return (PN_stdfloat)_node.m_im;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNodeElement::get_area
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat BulletSoftBodyNodeElement::
get_area() const {

  return (PN_stdfloat)_node.m_area;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSoftBodyNodeElement::is_attached
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int BulletSoftBodyNodeElement::
is_attached() const {

  return (PN_stdfloat)_node.m_battach;
}

