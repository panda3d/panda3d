/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletRigidBodyNode.cxx
 * @author enn0x
 * @date 2010-11-19
 */

#include "bulletRigidBodyNode.h"

#include "config_bullet.h"

#include "bulletShape.h"
#include "bulletWorld.h"

TypeHandle BulletRigidBodyNode::_type_handle;

/**
 *
 */
BulletRigidBodyNode::
BulletRigidBodyNode(const char *name) : BulletBodyNode(name) {
  // Mass properties
  btScalar mass(0.0);
  btVector3 inertia(0, 0, 0);

  // construction info
  btRigidBody::btRigidBodyConstructionInfo ci(mass, &_motion, _shape, inertia);

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

/**
 * Do not call the copy constructor directly; instead, use make_copy() or
 * copy_subgraph() to make a copy of a node.
 */
BulletRigidBodyNode::
BulletRigidBodyNode(const BulletRigidBodyNode &copy) :
  BulletBodyNode(copy)
{
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _motion = copy._motion;
  _rigid = new btRigidBody(*copy._rigid);
  _rigid->setUserPointer(this);
  _rigid->setCollisionShape(_shape);
  _rigid->setMotionState(&_motion);
}

/**
 * Returns a newly-allocated PandaNode that is a shallow copy of this one.  It
 * will be a different pointer, but its internal data may or may not be shared
 * with that of the original PandaNode.  No children will be copied.
 */
PandaNode *BulletRigidBodyNode::
make_copy() const {

  return new BulletRigidBodyNode(*this);
}

/**
 *
 */
void BulletRigidBodyNode::
output(std::ostream &out) const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  BulletBodyNode::do_output(out);

  out << " mass=" << do_get_mass();
}

/**
 *
 */
btCollisionObject *BulletRigidBodyNode::
get_object() const {

  return _rigid;
}

/**
 * Hook which should be called whenever the total shape of a body changed.
 * Used for example to update the mass properties (inertia) of a rigid body.
 * The default implementation does nothing.
 */
void BulletRigidBodyNode::
do_shape_changed() {

  do_set_mass(do_get_mass());
  do_transform_changed();
}

/**
 * Sets the mass of a rigid body.  This also modifies the inertia, which is
 * automatically computed from the shape of the body.  Setting a value of zero
 * for mass will make the body static.  A value of zero can be considered an
 * infinite mass.
 * Assumes the lock(bullet global lock) is held by the caller
 */
void BulletRigidBodyNode::
do_set_mass(PN_stdfloat mass) {

  btScalar bt_mass = mass;
  btVector3 bt_inertia(0.0, 0.0, 0.0);

  if (bt_mass > 0.0 && !_shapes.empty()) {
    _rigid->getCollisionShape()->calculateLocalInertia(bt_mass, bt_inertia);
  }

  _rigid->setMassProps(bt_mass, bt_inertia);
  _rigid->updateInertiaTensor();
}

/**
 * Sets the mass of a rigid body.  This also modifies the inertia, which is
 * automatically computed from the shape of the body.  Setting a value of zero
 * for mass will make the body static.  A value of zero can be considered an
 * infinite mass.
 */
void BulletRigidBodyNode::
set_mass(PN_stdfloat mass) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  do_set_mass(mass);
}

/**
 * Returns the total mass of a rigid body.  A value of zero means that the
 * body is staic, i.e.  has an infinite mass.
 * Assumes the lock(bullet global lock) is held by the caller
 */
PN_stdfloat BulletRigidBodyNode::
do_get_mass() const {

  btScalar inv_mass = _rigid->getInvMass();
  btScalar mass = (inv_mass == btScalar(0.0)) ? btScalar(0.0) : btScalar(1.0) / inv_mass;

  return mass;
}

/**
 * Returns the total mass of a rigid body.  A value of zero means that the
 * body is staic, i.e.  has an infinite mass.
 */
PN_stdfloat BulletRigidBodyNode::
get_mass() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return do_get_mass();
}


/**
 * Returns the inverse mass of a rigid body.
 */
PN_stdfloat BulletRigidBodyNode::
get_inv_mass() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_rigid->getInvMass();
}

/**
 * Sets the inertia of a rigid body.  Inertia is given as a three-component
 * vector.  A component value of zero means infinite inertia along this
 * direction.  Setting the intertia will override the value which is
 * automatically calculated from the rigid bodies shape.  However, it is
 * possible that automatic calculation of intertia is trigger after calling
 * this method, and thus overwriting the explicitly set value again.  This
 * happens when: (a) the mass is set after the inertia.  (b) a shape is added
 * or removed from the body.  (c) the scale of the body changed.
 */
void BulletRigidBodyNode::
set_inertia(const LVecBase3 &inertia) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  btVector3 inv_inertia(
    inertia.get_x() == 0.0 ? btScalar(0.0) : btScalar(1.0 / inertia.get_x()),
    inertia.get_y() == 0.0 ? btScalar(0.0) : btScalar(1.0 / inertia.get_y()),
    inertia.get_z() == 0.0 ? btScalar(0.0) : btScalar(1.0 / inertia.get_z())
    );

  _rigid->setInvInertiaDiagLocal(inv_inertia);
  _rigid->updateInertiaTensor();
}

/**
 * Returns the inertia of the rigid body.  Inertia is given as a three
 * component vector.  A component value of zero means infinite inertia along
 * this direction.
 */
LVector3 BulletRigidBodyNode::
get_inertia() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  btVector3 inv_inertia = _rigid->getInvInertiaDiagLocal();
  LVector3 inertia(
    inv_inertia.x() == btScalar(0.0) ? 0.0 : 1.0 / inv_inertia.x(),
    inv_inertia.y() == btScalar(0.0) ? 0.0 : 1.0 / inv_inertia.y(),
    inv_inertia.z() == btScalar(0.0) ? 0.0 : 1.0 / inv_inertia.z()
    );

  return inertia;
}

/**
 *
 */
LVector3 BulletRigidBodyNode::
get_inv_inertia_diag_local() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LVector3(_rigid->getInvInertiaDiagLocal());
}

/**
 *
 */
LMatrix3 BulletRigidBodyNode::
get_inv_inertia_tensor_world() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btMatrix3x3_to_LMatrix3(_rigid->getInvInertiaTensorWorld());
}

/**
 *
 */
void BulletRigidBodyNode::
apply_force(const LVector3 &force, const LPoint3 &pos) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv_always(!force.is_nan());
  nassertv_always(!pos.is_nan());

  _rigid->applyForce(LVecBase3_to_btVector3(force),
                     LVecBase3_to_btVector3(pos));
}

/**
 *
 */
void BulletRigidBodyNode::
apply_central_force(const LVector3 &force) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv_always(!force.is_nan());

  _rigid->applyCentralForce(LVecBase3_to_btVector3(force));
}

/**
 *
 */
void BulletRigidBodyNode::
apply_torque(const LVector3 &torque) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv_always(!torque.is_nan());

  _rigid->applyTorque(LVecBase3_to_btVector3(torque));
}

/**
 *
 */
void BulletRigidBodyNode::
apply_torque_impulse(const LVector3 &torque) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv_always(!torque.is_nan());

  _rigid->applyTorqueImpulse(LVecBase3_to_btVector3(torque));
}

/**
 *
 */
void BulletRigidBodyNode::
apply_impulse(const LVector3 &impulse, const LPoint3 &pos) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv_always(!impulse.is_nan());
  nassertv_always(!pos.is_nan());

  _rigid->applyImpulse(LVecBase3_to_btVector3(impulse),
                       LVecBase3_to_btVector3(pos));
}

/**
 *
 */
void BulletRigidBodyNode::
apply_central_impulse(const LVector3 &impulse) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv_always(!impulse.is_nan());

  _rigid->applyCentralImpulse(LVecBase3_to_btVector3(impulse));
}

/**
 * Assumes the lock(bullet global lock) is held by the caller
 */
void BulletRigidBodyNode::
do_transform_changed() {

  if (_motion.sync_disabled()) return;

  NodePath np = NodePath::any_path((PandaNode *)this);
  CPT(TransformState) ts = np.get_net_transform();

  // For kinematic bodies Bullet will query the transform via
  // Motionstate::getWorldTransform.  Therefor we need to store the new
  // transform within the motion state.  For dynamic bodies we need to store
  // the net scale within the motion state, since Bullet might update the
  // transform via MotionState::setWorldTransform.
  _motion.set_net_transform(ts);

  // For dynamic or static bodies we directly apply the new transform.
  if (!(get_object()->isKinematicObject())) {
    btTransform trans = TransformState_to_btTrans(ts);
    _rigid->setCenterOfMassTransform(trans);
  }

  // Rescale all shapes, but only if the new transform state has a scale, and
  // this scale differes from the current scale.
  if (ts->has_scale()) {
    btVector3 new_scale = LVecBase3_to_btVector3(ts->get_scale());
    btVector3 current_scale = _shape->getLocalScaling();
    btVector3 current_scale_inv(1.0/current_scale.x(), 1.0/current_scale.y(), 1.0/current_scale.z());

    if (new_scale != current_scale) {
      _shape->setLocalScaling(current_scale_inv);
      _shape->setLocalScaling(new_scale);
    }
  }

  // Activate the body if it has been sleeping
  if (!_rigid->isActive()) {
    _rigid->activate(true);
  }
}

/**
 *
 */
void BulletRigidBodyNode::
parents_changed() {

  if (_motion.sync_disabled()) return;

  if (get_num_parents() > 0) {
    LightMutexHolder holder(BulletWorld::get_global_lock());
    do_transform_changed();
  }
}

/**
 *
 */
void BulletRigidBodyNode::
transform_changed() {

  if (_motion.sync_disabled()) return;

  LightMutexHolder holder(BulletWorld::get_global_lock());

  do_transform_changed();
}

/**
 * Assumes the lock(bullet global lock) is held by the caller
 */
void BulletRigidBodyNode::
do_sync_p2b() {

  if (get_object()->isKinematicObject()) {
    do_transform_changed();
  }
}

/**
 * Assumes the lock(bullet global lock) is held by the caller
 */
void BulletRigidBodyNode::
do_sync_b2p() {

  _motion.sync_b2p((PandaNode *)this);
}

/**
 *
 */
LVector3 BulletRigidBodyNode::
get_linear_velocity() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LVector3(_rigid->getLinearVelocity());
}

/**
 *
 */
LVector3 BulletRigidBodyNode::
get_angular_velocity() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LVector3(_rigid->getAngularVelocity());
}

/**
 *
 */
void BulletRigidBodyNode::
set_linear_velocity(const LVector3 &velocity) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv_always(!velocity.is_nan());

  _rigid->setLinearVelocity(LVecBase3_to_btVector3(velocity));
}

/**
 *
 */
void BulletRigidBodyNode::
set_angular_velocity(const LVector3 &velocity) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv_always(!velocity.is_nan());

  _rigid->setAngularVelocity(LVecBase3_to_btVector3(velocity));
}

/**
 *
 */
void BulletRigidBodyNode::
set_linear_damping(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _rigid->setDamping(value, _rigid->getAngularDamping());
}

/**
 *
 */
void BulletRigidBodyNode::
set_angular_damping(PN_stdfloat value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _rigid->setDamping(_rigid->getLinearDamping(), value);
}

/**
 *
 */
PN_stdfloat BulletRigidBodyNode::
get_linear_damping() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_rigid->getLinearDamping();
}

/**
 *
 */
PN_stdfloat BulletRigidBodyNode::
get_angular_damping() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (PN_stdfloat)_rigid->getAngularDamping();
}

/**
 *
 */
void BulletRigidBodyNode::
clear_forces() {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _rigid->clearForces();
}

/**
 *
 */
PN_stdfloat BulletRigidBodyNode::
get_linear_sleep_threshold() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _rigid->getLinearSleepingThreshold();
}

/**
 *
 */
PN_stdfloat BulletRigidBodyNode::
get_angular_sleep_threshold() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _rigid->getAngularSleepingThreshold();
}

/**
 *
 */
void BulletRigidBodyNode::
set_linear_sleep_threshold(PN_stdfloat threshold) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _rigid->setSleepingThresholds(threshold, _rigid->getAngularSleepingThreshold());
}

/**
 *
 */
void BulletRigidBodyNode::
set_angular_sleep_threshold(PN_stdfloat threshold) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _rigid->setSleepingThresholds(_rigid->getLinearSleepingThreshold(), threshold);
}

/**
 *
 */
void BulletRigidBodyNode::
set_gravity(const LVector3 &gravity) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv_always(!gravity.is_nan());

  _rigid->setGravity(LVecBase3_to_btVector3(gravity));
}

/**
 *
 */
LVector3 BulletRigidBodyNode::
get_gravity() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LVector3(_rigid->getGravity());
}

/**
 *
 */
LVector3 BulletRigidBodyNode::
get_linear_factor() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LVector3(_rigid->getLinearFactor());
}

/**
 *
 */
LVector3 BulletRigidBodyNode::
get_angular_factor() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LVector3(_rigid->getAngularFactor());
}

/**
 *
 */
void BulletRigidBodyNode::
set_linear_factor(const LVector3 &factor) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _rigid->setLinearFactor(LVecBase3_to_btVector3(factor));
}

/**
 *
 */
void BulletRigidBodyNode::
set_angular_factor(const LVector3 &factor) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _rigid->setAngularFactor(LVecBase3_to_btVector3(factor));
}

/**
 *
 */
LVector3 BulletRigidBodyNode::
get_total_force() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LVector3(_rigid->getTotalForce());
}

/**
 *
 */
LVector3 BulletRigidBodyNode::
get_total_torque() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LVector3(_rigid->getTotalTorque());
}

/**
 *
 */
BulletRigidBodyNode::MotionState::
MotionState() {

  _trans.setIdentity();
  _disabled = false;
  _dirty = false;
  _was_dirty = false;
}

/**
 *
 */
void BulletRigidBodyNode::MotionState::
getWorldTransform(btTransform &trans) const {

  trans = _trans;
}

/**
 *
 */
void BulletRigidBodyNode::MotionState::
setWorldTransform(const btTransform &trans) {

  _trans = trans;
  _dirty = true;
  _was_dirty = true;
}

/**
 *
 */
void BulletRigidBodyNode::MotionState::
sync_b2p(PandaNode *node) {

  if (!_dirty) return;

  NodePath np = NodePath::any_path(node);
  LPoint3 p = btVector3_to_LPoint3(_trans.getOrigin());
  LQuaternion q = btQuat_to_LQuaternion(_trans.getRotation());

  _disabled = true;
  np.set_pos_quat(NodePath(), p, q);
  _disabled = false;
  _dirty = false;
}

/**
 * This method stores the global transform within the Motionstate.  It is
 * called from BulletRigidBodyNode::transform_changed(). For kinematic bodies
 * the global transform is required since Bullet queries the body transform
 * via MotionState::getGlobalStranform(). For dynamic bodies the global scale
 * is required, since Bullet will overwrite the member _trans by calling
 * MotionState::setGlobalTransform.
 */
void BulletRigidBodyNode::MotionState::
set_net_transform(const TransformState *ts) {

  nassertv(ts);

  _trans = TransformState_to_btTrans(ts);
}

/**
 *
 */
bool BulletRigidBodyNode::MotionState::
sync_disabled() const {

  return _disabled;
}

/**
 *
 */
bool BulletRigidBodyNode::MotionState::
pick_dirty_flag() {

  bool rc = _was_dirty;
  _was_dirty = false;
  return rc;
}

/**
 * Returns TRUE if the transform of the rigid body has changed at least once
 * since the last call to this method.
 */
bool BulletRigidBodyNode::
pick_dirty_flag() {

  return _motion.pick_dirty_flag();
}

/**
 * Tells the BamReader how to create objects of type BulletRigidBodyNode.
 */
void BulletRigidBodyNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void BulletRigidBodyNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  BulletBodyNode::write_datagram(manager, dg);

  dg.add_stdfloat(get_mass());
  dg.add_stdfloat(get_linear_damping());
  dg.add_stdfloat(get_angular_damping());
  dg.add_stdfloat(get_linear_sleep_threshold());
  dg.add_stdfloat(get_angular_sleep_threshold());
  get_gravity().write_datagram(dg);
  get_linear_factor().write_datagram(dg);
  get_angular_factor().write_datagram(dg);
  // dynamic state (?)
  get_linear_velocity().write_datagram(dg);
  get_angular_velocity().write_datagram(dg);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * this type is encountered in the Bam file.  It should create the rigid body
 * and extract its information from the file.
 */
TypedWritable *BulletRigidBodyNode::
make_from_bam(const FactoryParams &params) {
  BulletRigidBodyNode *param = new BulletRigidBodyNode;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  param->fillin(scan, manager);

  return param;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new BulletRigidBodyNode.
 */
void BulletRigidBodyNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  BulletBodyNode::fillin(scan, manager);

  set_mass(scan.get_stdfloat());
  set_linear_damping(scan.get_stdfloat());
  set_angular_damping(scan.get_stdfloat());
  set_linear_sleep_threshold(scan.get_stdfloat());
  set_angular_sleep_threshold(scan.get_stdfloat());

  LVector3 gravity, linear_factor, angular_factor;
  gravity.read_datagram(scan);
  linear_factor.read_datagram(scan);
  angular_factor.read_datagram(scan);

  set_gravity(gravity);
  set_linear_factor(linear_factor);
  set_angular_factor(angular_factor);
}
