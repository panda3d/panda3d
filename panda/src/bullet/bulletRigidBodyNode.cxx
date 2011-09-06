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

  // Synchronised transform
  _sync = TransformState::make_identity();
  _sync_disable = false;

  // Mass properties
  btScalar mass(0.0);
  btVector3 inertia(0, 0, 0);

  // Motion state and construction info
  MotionState *motion = new MotionState(_sync);
  btRigidBody::btRigidBodyConstructionInfo ci(mass, motion, _shape, inertia);

  // Additional damping
  if (bullet_additional_damping) {
    ci.m_additionalDamping = true;
    ci.m_additionalDampingFactor = bullet_additional_damping_linear_factor;
    ci.m_additionalLinearDampingThresholdSqr = bullet_additional_damping_linear_threshold;
    ci.m_additionalAngularDampingFactor = bullet_additional_damping_angular_factor;
    ci.m_additionalAngularDampingThresholdSqr = bullet_additional_damping_angular_threshold;
  }

  // Rigid body
  _rigid = new btRigidBody(ci);
  _rigid->setUserPointer(this);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
output(ostream &out) const {

  BulletBodyNode::output(out);

  out << " mass=" << get_mass();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::get_object
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
btCollisionObject *BulletRigidBodyNode::
get_object() const {

  return _rigid;
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

  btScalar bt_mass = mass;
  btVector3 bt_inertia(0.0, 0.0, 0.0);

  if (bt_mass > 0.0) {
    _rigid->getCollisionShape()->calculateLocalInertia(bt_mass, bt_inertia);
  }

  _rigid->setMassProps(bt_mass, bt_inertia);
  _rigid->updateInertiaTensor();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::get_mass
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
float BulletRigidBodyNode::
get_mass() const {

  btScalar inv_mass = _rigid->getInvMass();
  btScalar mass = inv_mass == btScalar(0.0) ? btScalar(0.0) : btScalar(1.0) / inv_mass;

  return mass;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::set_inertia
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
set_inertia(const LVecBase3f &inertia) {

  btVector3 inv_inertia(
    inertia.get_x() == 0.0 ? btScalar(0.0) : btScalar(1.0 / inertia.get_x()),
    inertia.get_y() == 0.0 ? btScalar(0.0) : btScalar(1.0 / inertia.get_y()),
    inertia.get_z() == 0.0 ? btScalar(0.0) : btScalar(1.0 / inertia.get_z())
    );

  _rigid->setInvInertiaDiagLocal(inv_inertia);
  _rigid->updateInertiaTensor();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::get_inertia
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LVector3f BulletRigidBodyNode::
get_inertia() const {

  btVector3 inv_inertia = _rigid->getInvInertiaDiagLocal();
  LVector3f inertia(
    inv_inertia.x() == btScalar(0.0) ? 0.0 : 1.0 / inv_inertia.x(),
    inv_inertia.y() == btScalar(0.0) ? 0.0 : 1.0 / inv_inertia.y(),
    inv_inertia.z() == btScalar(0.0) ? 0.0 : 1.0 / inv_inertia.z()
    );

  return inertia;
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

  _rigid->applyForce(LVecBase3f_to_btVector3(force),
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

  _rigid->applyCentralForce(LVecBase3f_to_btVector3(force));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::apply_torque
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
apply_torque(const LVector3f &torque) {

  nassertv_always(!torque.is_nan());

  _rigid->applyTorque(LVecBase3f_to_btVector3(torque));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::apply_torque_impulse
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
apply_torque_impulse(const LVector3f &torque) {

  nassertv_always(!torque.is_nan());

  _rigid->applyTorqueImpulse(LVecBase3f_to_btVector3(torque));
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

  _rigid->applyImpulse(LVecBase3f_to_btVector3(impulse),
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

  _rigid->applyCentralImpulse(LVecBase3f_to_btVector3(impulse));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::transform_changed
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
transform_changed() {

  if (_sync_disable) return;

  NodePath np = NodePath::any_path((PandaNode *)this);
  CPT(TransformState) ts = np.get_net_transform();

  LMatrix4f m_sync = _sync->get_mat();
  LMatrix4f m_ts = ts->get_mat();

  if (!m_sync.almost_equal(m_ts)) {
    _sync = ts;

    btTransform trans = TransformState_to_btTrans(ts);
    _rigid->setCenterOfMassTransform(trans);

    if (ts->has_scale()) {
      LVecBase3f scale = ts->get_scale();
      if (!scale.almost_equal(LVecBase3f(1.0f, 1.0f, 1.0f))) {
        for (int i=0; i<get_num_shapes(); i++) {
          PT(BulletShape) shape = _shapes[i];
          shape->set_local_scale(scale);
        }

        shape_changed();
      }
    }

    // Activate the body if it has been sleeping
    if (!_rigid->isActive()) {
      _rigid->activate(true);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::sync_p2b
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
sync_p2b() {

  transform_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::sync_b2p
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
sync_b2p() {

  NodePath np = NodePath::any_path((PandaNode *)this);
  LVecBase3f scale = np.get_net_transform()->get_scale();

  btTransform trans = _rigid->getWorldTransform();
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
//     Function: BulletRigidBodyNode::get_linear_velocity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LVector3f BulletRigidBodyNode::
get_linear_velocity() const {

  return btVector3_to_LVector3f(_rigid->getLinearVelocity());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::get_angular_velocity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LVector3f BulletRigidBodyNode::
get_angular_velocity() const {

  return btVector3_to_LVector3f(_rigid->getAngularVelocity());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::set_linear_velocity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
set_linear_velocity(const LVector3f &velocity) {

  nassertv_always(!velocity.is_nan());

  _rigid->setLinearVelocity(LVecBase3f_to_btVector3(velocity));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::set_angular_velocity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
set_angular_velocity(const LVector3f &velocity) {

  nassertv_always(!velocity.is_nan());

  _rigid->setAngularVelocity(LVecBase3f_to_btVector3(velocity));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::clear_forces
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
clear_forces() {

  _rigid->clearForces();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::get_linear_sleep_threshold
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
float BulletRigidBodyNode::
get_linear_sleep_threshold() const {

  return _rigid->getLinearSleepingThreshold();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::get_angular_sleep_threshold
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
float BulletRigidBodyNode::
get_angular_sleep_threshold() const {

  return _rigid->getAngularSleepingThreshold();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::set_linear_sleep_threshold
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
set_linear_sleep_threshold(float threshold) {

  _rigid->setSleepingThresholds(_rigid->getLinearSleepingThreshold(), threshold);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::set_angular_sleep_threshold
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
set_angular_sleep_threshold(float threshold) {

  _rigid->setSleepingThresholds(threshold, _rigid->getAngularSleepingThreshold());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::set_gravity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
set_gravity(const LVector3f &gravity) {

  nassertv_always(!gravity.is_nan());

  _rigid->setGravity(LVecBase3f_to_btVector3(gravity));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::get_gravity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LVector3f BulletRigidBodyNode::
get_gravity() const {

  return btVector3_to_LVector3f(_rigid->getGravity());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::set_linear_factor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
set_linear_factor(const LVector3f &factor) {

  _rigid->setLinearFactor(LVecBase3f_to_btVector3(factor));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::set_angular_factor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::
set_angular_factor(const LVector3f &factor) {

  _rigid->setAngularFactor(LVecBase3f_to_btVector3(factor));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::MotionState::getWorldTransform
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::MotionState::
getWorldTransform(btTransform &trans) const {

  trans = TransformState_to_btTrans(_sync);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRigidBodyNode::MotionState::setWorldTransform
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void BulletRigidBodyNode::MotionState::
setWorldTransform(const btTransform &trans) {

}

