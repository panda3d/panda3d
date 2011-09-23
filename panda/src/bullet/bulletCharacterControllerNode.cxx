// Filename: bulletCharacterControllerNode.cxx
// Created by:  enn0x (21Nov10)
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

#include "bulletCharacterControllerNode.h"

TypeHandle BulletCharacterControllerNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BulletCharacterControllerNode::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletCharacterControllerNode::
BulletCharacterControllerNode(BulletShape *shape, float step_height, const char *name) : PandaNode(name) {

  // Synchronised transform
  _sync = TransformState::make_identity();
  _sync_disable = false;

  // Initial transform
  btTransform trans = btTransform::getIdentity();

  // Get convex shape (for ghost object)
  if (!shape->is_convex()) {
    bullet_cat.error() << "a convex shape is required!" << endl;
    return;
  }

  btConvexShape *convex = (btConvexShape *)(shape->ptr());

  // Ghost object
  _ghost = new btPairCachingGhostObject();
  _ghost->setUserPointer(this);

  _ghost->setWorldTransform(trans);
  _ghost->setInterpolationWorldTransform(trans);
  _ghost->setCollisionShape(convex);
  _ghost->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);

  // Up axis
  _up = get_default_up_axis();

  // Initialise movement
  _linear_velocity_is_local = false;
  _linear_velocity.set(0.0f, 0.0f, 0.0f);
  _angular_velocity = 0.0f;

  // Character controller
  _character = new btKinematicCharacterController(_ghost, convex, step_height, _up);
  _character->setGravity((btScalar)9.81f);

  // Retain a pointer to the shape
  _shape = shape;

  // Default collide mask
  set_into_collide_mask(CollideMask::all_on());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCharacterControllerNode::get_legal_collide_mask
//       Access: Public, virtual
//  Description: Returns the subset of CollideMask bits that may be
//               set for this particular type of PandaNode.  For 
//               CharacterControllerNodes this returns all bits on.
////////////////////////////////////////////////////////////////////
CollideMask BulletCharacterControllerNode::
get_legal_collide_mask() const {

  return CollideMask::all_on();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCharacterControllerNode::safe_to_flatten
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to flatten out
//               this particular kind of Node by duplicating
//               instances, false otherwise (for instance, a Camera
//               cannot be safely flattened, because the Camera
//               pointer itself is meaningful).
////////////////////////////////////////////////////////////////////
bool BulletCharacterControllerNode::
safe_to_flatten() const {

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCharacterControllerNode::safe_to_modify_transform
//       Access: Public, Virtual
//  Description: Returns true if it is safe to automatically adjust
//               the transform on this kind of node.  Usually, this is
//               only a bad idea if the user expects to find a
//               particular transform on the node.
//
//               ModelNodes with the preserve_transform flag set are
//               presently the only kinds of nodes that should not
//               have their transform even adjusted.
////////////////////////////////////////////////////////////////////
bool BulletCharacterControllerNode::
safe_to_modify_transform() const {

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCharacterControllerNode::safe_to_combine
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to combine this
//               particular kind of PandaNode with other kinds of
//               PandaNodes of compatible type, adding children or
//               whatever.  For instance, an LODNode should not be
//               combined with any other PandaNode, because its set of
//               children is meaningful.
////////////////////////////////////////////////////////////////////
bool BulletCharacterControllerNode::
safe_to_combine() const {

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCharacterControllerNode::safe_to_combine_children
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to combine the
//               children of this PandaNode with each other.  For
//               instance, an LODNode's children should not be
//               combined with each other, because the set of children
//               is meaningful.
////////////////////////////////////////////////////////////////////
bool BulletCharacterControllerNode::
safe_to_combine_children() const {

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCharacterControllerNode::safe_to_flatten_below
//       Access: Public, Virtual
//  Description: Returns true if a flatten operation may safely
//               continue past this node, or false if nodes below this
//               node may not be molested.
////////////////////////////////////////////////////////////////////
bool BulletCharacterControllerNode::
safe_to_flatten_below() const {

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCharacterControllerNode::safe_to_transform
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to transform
//               this particular kind of Node by calling the xform()
//               method, false otherwise.  For instance, it's usually
//               a bad idea to attempt to xform a Character.
////////////////////////////////////////////////////////////////////
bool BulletCharacterControllerNode::
safe_to_transform() const {

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCharacterControllerNode::set_linear_velocity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletCharacterControllerNode::
set_linear_velocity(const LVector3f &velocity, bool is_local) {

  nassertv(!velocity.is_nan());

  _linear_velocity = velocity;
  _linear_velocity_is_local = is_local;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCharacterControllerNode::set_angular_velocity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletCharacterControllerNode::
set_angular_velocity(float omega) {

  _angular_velocity = omega;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCharacterControllerNode::sync_p2b
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void BulletCharacterControllerNode::
sync_p2b(float dt) {

  // Synchronise global transform
  transform_changed();

  // Angular rotation
  btScalar angle = dt * deg_2_rad(_angular_velocity);

  btMatrix3x3 m = _ghost->getWorldTransform().getBasis();
  btVector3 up = m[_up];

  m *= btMatrix3x3(btQuaternion(up, angle));

  _ghost->getWorldTransform().setBasis(m);

  // Linear movement
  btVector3 v;
  if (_linear_velocity_is_local) {
    btTransform xform = _ghost->getWorldTransform();
    xform.setOrigin(btVector3(0.0f, 0.0f, 0.0f));
    v = xform(LVecBase3f_to_btVector3(_linear_velocity));
  }
  else {
    v = LVecBase3f_to_btVector3(_linear_velocity);
  }

  //_character->setVelocityForTimeInterval(v, dt);
  _character->setWalkDirection(v * dt);
  //_character->setWalkDirection(v);
  _angular_velocity = 0.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCharacterControllerNode::sync_b2p
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void BulletCharacterControllerNode::
sync_b2p() {

  NodePath np = NodePath::any_path((PandaNode *)this);
  LVecBase3f scale = np.get_net_transform()->get_scale();

  btTransform trans = _ghost->getWorldTransform();
  CPT(TransformState) ts = btTrans_to_TransformState(trans, scale);

  LMatrix4f m_sync = _sync->get_mat();
  LMatrix4f m_ts = ts->get_mat();

  if (!m_sync.almost_equal(m_ts)) {
    _sync = ts;
    _sync_disable = true;
    np.set_transform(NodePath(), ts);
    _sync_disable = false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCharacterControllerNode::transform_changed
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void BulletCharacterControllerNode::
transform_changed() {

  if (_sync_disable) return;

  NodePath np = NodePath::any_path((PandaNode *)this);
  CPT(TransformState) ts = np.get_net_transform();

  LMatrix4f m_sync = _sync->get_mat();
  LMatrix4f m_ts = ts->get_mat();

  if (!m_sync.almost_equal(m_ts)) {
    _sync = ts;

    // Get translation, heading and scale
    LPoint3f pos = ts->get_pos();
    float heading = ts->get_hpr().get_x();
    LVecBase3f scale = ts->get_scale();

    // Set translation
    _character->warp(LVecBase3f_to_btVector3(pos));

    // Set Heading
    btMatrix3x3 m = _ghost->getWorldTransform().getBasis();
    btVector3 up = m[_up];

    m = btMatrix3x3(btQuaternion(up, heading));

    _ghost->getWorldTransform().setBasis(m);

    // Set scale
    _shape->set_local_scale(scale);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCharacterControllerNode::get_shape
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletShape *BulletCharacterControllerNode::
get_shape() const {

  return _shape;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCharacterControllerNode::is_on_ground
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
bool BulletCharacterControllerNode::
is_on_ground() const {

  return _character->onGround();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCharacterControllerNode::can_jump
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
bool BulletCharacterControllerNode::
can_jump() const {

  return _character->canJump();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCharacterControllerNode::do_jump
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletCharacterControllerNode::
do_jump() {

  _character->jump();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCharacterControllerNode::set_fall_speed
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletCharacterControllerNode::
set_fall_speed(float fall_speed) {

  _character->setFallSpeed(fall_speed);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCharacterControllerNode::set_jump_speed
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletCharacterControllerNode::
set_jump_speed(float jump_speed) {

  _character->setJumpSpeed(jump_speed);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCharacterControllerNode::set_max_jump_height
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletCharacterControllerNode::
set_max_jump_height(float max_jump_height) {

  _character->setMaxJumpHeight(max_jump_height);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCharacterControllerNode::set_max_slope
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletCharacterControllerNode::
set_max_slope(float max_slope) {

  _character->setMaxSlope(max_slope);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCharacterControllerNode::get_max_slope
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
float BulletCharacterControllerNode::
get_max_slope() const {

  return _character->getMaxSlope();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCharacterControllerNode::get_gravity
//  Description:
////////////////////////////////////////////////////////////////////
float BulletCharacterControllerNode::
get_gravity() const {

  return _character->getGravity();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCharacterControllerNode::set_gravity
//  Description:
////////////////////////////////////////////////////////////////////
void BulletCharacterControllerNode::
set_gravity(float gravity) {

  _character->setGravity((btScalar) gravity);
}


////////////////////////////////////////////////////////////////////
//     Function: BulletCharacterControllerNode::set_use_ghost_sweep_test
//  Description:
////////////////////////////////////////////////////////////////////
void BulletCharacterControllerNode::
set_use_ghost_sweep_test(bool value) {

  return _character->setUseGhostSweepTest(value);
}

