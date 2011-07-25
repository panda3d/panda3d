// Filename: bulletBodyNode.I
// Created by:  enn0x (19Nov10)
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
//     Function: BulletBodyNode::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletBodyNode::
~BulletBodyNode() {

  if (_shape && (
      _shape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE || 
      _shape->getShapeType() == EMPTY_SHAPE_PROXYTYPE)) {
    delete _shape;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::notify_collisions
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletBodyNode::
notify_collisions(bool value) {

  set_collision_flag(btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK, value);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::notifies_collisions
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool BulletBodyNode::
notifies_collisions() const {

  return get_collision_flag(btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::set_collision_response
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletBodyNode::
set_collision_response(bool value) {

  set_collision_flag(btCollisionObject::CF_NO_CONTACT_RESPONSE, !value);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::get_collision_response
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool BulletBodyNode::
get_collision_response() const {

  return !get_collision_flag(btCollisionObject::CF_NO_CONTACT_RESPONSE);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::set_collision_flag
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletBodyNode::
set_collision_flag(int flag, bool value) {

  int flags = get_object()->getCollisionFlags();

  if (value == true) {
    flags |= flag;
  }
  else {
    flags &= ~(flag);
  }

  get_object()->setCollisionFlags(flags);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::get_collision_flag
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool BulletBodyNode::
get_collision_flag(int flag) const {

  return (get_object()->getCollisionFlags() & flag) ? true : false;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::is_static
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool BulletBodyNode::
is_static() const {

  return get_object()->isStaticObject();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::is_kinematic
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool BulletBodyNode::
is_kinematic() const {

  return get_object()->isKinematicObject();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::set_static
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletBodyNode::
set_static(bool value) {

  set_collision_flag(btCollisionObject::CF_STATIC_OBJECT, value);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::set_kinematic
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletBodyNode::
set_kinematic(bool value) {

  set_collision_flag(btCollisionObject::CF_KINEMATIC_OBJECT, value);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::get_restitution
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float BulletBodyNode::
get_restitution() const {

  return get_object()->getRestitution();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::set_restitution
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletBodyNode::
set_restitution(float restitution) {

  return get_object()->setRestitution(restitution);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::get_friction
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float BulletBodyNode::
get_friction() const {

  return get_object()->getFriction();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::set_friction
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletBodyNode::
set_friction(float friction) {

  return get_object()->setFriction(friction);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::has_anisotropic_friction
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool BulletBodyNode::
has_anisotropic_friction() const {

  return get_object()->hasAnisotropicFriction();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::get_num_shapes
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int BulletBodyNode::
get_num_shapes() const {

  return _shapes.size();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::get_shape
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletShape *BulletBodyNode::
get_shape(int idx) const {

  nassertr(idx >= 0 && idx < (int)_shapes.size(), NULL);
  return _shapes[idx];
}

