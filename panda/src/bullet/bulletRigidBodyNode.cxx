// Filename: bulletRigidBodyNode.cxx
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

#include "bulletRigidBodyNode.h"
#include "bulletShape.h"

TypeHandle BulletRigidBodyNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletRigidBodyNode::
BulletRigidBodyNode(const char *name) : BulletBodyNode(name) {

  // Setup motion state
  _motion = new MotionState(this);

  // Setup mass properties
  btScalar mass(0.0);
  btVector3 inertia(0, 0, 0);

  btRigidBody::btRigidBodyConstructionInfo ci(mass, _motion, _shape, inertia);

  // Enable additional damping
  if (bullet_additional_damping) {
    ci.m_additionalDamping = true;
    ci.m_additionalDampingFactor = bullet_additional_damping_linear_factor;
    ci.m_additionalLinearDampingThresholdSqr = bullet_additional_damping_linear_threshold;
    ci.m_additionalAngularDampingFactor = bullet_additional_damping_angular_factor;
    ci.m_additionalAngularDampingThresholdSqr = bullet_additional_damping_angular_threshold;
  }

  // Setup rigid body
  _body = new btRigidBody(ci);
  _body->setUserPointer(this);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::get_object
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
btCollisionObject *BulletRigidBodyNode::
get_object() const {

  return _body;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::shape_changed
//       Access: Published
//  Description: Hook which will be called whenever the total shape
//               of a body changed. Used for example to update
//               the mass properties (inertia) of a rigid body.
//               The default implementation does nothing.
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
shape_changed() {

  set_mass(get_mass());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::set_mass
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
set_mass(float mass) {

  btVector3 inertia(0.0f, 0.0f, 0.0f);

  if (mass > 0.0f) {
    _body->getCollisionShape()->calculateLocalInertia(mass, inertia);
  }

  _body->setMassProps(mass, inertia);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::get_mass
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
float BulletRigidBodyNode::
get_mass() const {

  btScalar invMass = _body->getInvMass();

  if (invMass == 0.0f) {
    return 0.0f;
  }
  else {
    return 1.0f / invMass;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::apply_force
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
apply_force(const LVector3f &force, const LPoint3f &pos) {

  nassertv_always(!force.is_nan());
  nassertv_always(!pos.is_nan());

  _body->applyForce(LVecBase3f_to_btVector3(force),
                    LVecBase3f_to_btVector3(pos));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::apply_central_force
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
apply_central_force(const LVector3f &force) {

  nassertv_always(!force.is_nan());

  _body->applyCentralForce(LVecBase3f_to_btVector3(force));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::apply_torque
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
apply_torque(const LVector3f &torque) {

  nassertv_always(!torque.is_nan());

  _body->applyTorque(LVecBase3f_to_btVector3(torque));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::apply_torque_impulse
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
apply_torque_impulse(const LVector3f &torque) {

  nassertv_always(!torque.is_nan());

  _body->applyTorqueImpulse(LVecBase3f_to_btVector3(torque));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::apply_impulse
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
apply_impulse(const LVector3f &impulse, const LPoint3f &pos) {

  nassertv_always(!impulse.is_nan());
  nassertv_always(!pos.is_nan());

  _body->applyImpulse(LVecBase3f_to_btVector3(impulse),
                      LVecBase3f_to_btVector3(pos));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::apply_central_impulse
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
apply_central_impulse(const LVector3f &impulse) {

  nassertv_always(!impulse.is_nan());

  _body->applyCentralImpulse(LVecBase3f_to_btVector3(impulse));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::parents_changed
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
parents_changed() {

  transform_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::transform_changed
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
transform_changed() {

  if (_disable_transform_changed) return;

  btTransform trans;
  get_node_transform(trans, this);
  _body->setWorldTransform(trans);
  _body->setInterpolationWorldTransform(trans);

  BulletBodyNode::transform_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::MotionState::getWorldTransform
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::MotionState::
getWorldTransform(btTransform &trans) const {

  get_node_transform(trans, _node);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::MotionState::setWorldTransform
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::MotionState::
setWorldTransform(const btTransform &trans) {

  if (trans.getOrigin().getX() != trans.getOrigin().getX()) {
    bullet_cat.error() << "setWorldTransform: trans is NAN!" << endl;
    return;
  }

  LVecBase3f scale = _node->get_transform()->get_scale();
  CPT(TransformState) ts = btTrans_to_TransformState(trans, scale);

  // Disable transform_changed callback
  _node->_disable_transform_changed = true;

  if (_node->get_num_parents() == 0) {
    _node->set_transform(ts);
  }
  else {
    NodePath np = NodePath::any_path(_node);
    np.set_transform(np.get_top(), ts);
  }

  // Re-enable transform_changed callback again
  _node->_disable_transform_changed = false;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::set_center_of_mass_pos
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
set_center_of_mass_pos(const LPoint3f &pos) {

  nassertv_always(!pos.is_nan());

  btTransform xform;
  xform.setIdentity();
  xform.setOrigin(LVecBase3f_to_btVector3(pos));

  _body->setCenterOfMassTransform(xform);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::get_center_of_mass_pos
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LPoint3f BulletRigidBodyNode::
get_center_of_mass_pos() const {

  return btVector3_to_LPoint3f(_body->getCenterOfMassPosition());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::get_linear_velocity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LVector3f BulletRigidBodyNode::
get_linear_velocity() const {

  return btVector3_to_LVector3f(_body->getLinearVelocity());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::get_angular_velocity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LVector3f BulletRigidBodyNode::
get_angular_velocity() const {

  return btVector3_to_LVector3f(_body->getAngularVelocity());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::set_linear_velocity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
set_linear_velocity(const LVector3f &velocity) {

  nassertv_always(!velocity.is_nan());

  _body->setLinearVelocity(LVecBase3f_to_btVector3(velocity));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::set_angular_velocity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
set_angular_velocity(const LVector3f &velocity) {

  nassertv_always(!velocity.is_nan());

  _body->setAngularVelocity(LVecBase3f_to_btVector3(velocity));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::clear_forces
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
clear_forces() {

  _body->clearForces();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::get_linear_sleep_threshold
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
float BulletRigidBodyNode::
get_linear_sleep_threshold() const {

  return _body->getLinearSleepingThreshold();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::get_angular_sleep_threshold
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
float BulletRigidBodyNode::
get_angular_sleep_threshold() const {

  return _body->getAngularSleepingThreshold();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::set_linear_sleep_threshold
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
set_linear_sleep_threshold(float threshold) {

  _body->setSleepingThresholds(_body->getLinearSleepingThreshold(), threshold);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::set_angular_sleep_threshold
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
set_angular_sleep_threshold(float threshold) {

  _body->setSleepingThresholds(threshold, _body->getAngularSleepingThreshold());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::set_gravity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
set_gravity(const LVector3f &gravity) {

  nassertv_always(!gravity.is_nan());

  _body->setGravity(LVecBase3f_to_btVector3(gravity));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::get_gravity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LVector3f BulletRigidBodyNode::
get_gravity() const {

  return btVector3_to_LVector3f(_body->getGravity());
}

