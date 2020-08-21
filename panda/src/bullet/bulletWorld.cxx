/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletWorld.cxx
 * @author enn0x
 * @date 2010-01-23
 */

#include "bulletWorld.h"

#include "config_bullet.h"

#include "bulletFilterCallbackData.h"
#include "bulletPersistentManifold.h"
#include "bulletShape.h"
#include "bulletSoftBodyWorldInfo.h"
#include "bulletTickCallbackData.h"

#include "collideMask.h"
#include "lightMutexHolder.h"

#define clamp(x, x_min, x_max) std::max(std::min(x, x_max), x_min)

using std::endl;
using std::istream;
using std::ostream;
using std::string;

TypeHandle BulletWorld::_type_handle;

PStatCollector BulletWorld::_pstat_physics("App:Bullet:DoPhysics");
PStatCollector BulletWorld::_pstat_simulation("App:Bullet:DoPhysics:Simulation");
PStatCollector BulletWorld::_pstat_p2b("App:Bullet:DoPhysics:SyncP2B");
PStatCollector BulletWorld::_pstat_b2p("App:Bullet:DoPhysics:SyncB2P");

PT(CallbackObject) bullet_contact_added_callback;

/**
 *
 */
BulletWorld::
BulletWorld() {

  // Init groups filter matrix
  for (size_t i = 0; i < 32; ++i) {
    _filter_cb2._collide[i].clear();
    _filter_cb2._collide[i].set_bit(i);
  }

  // Broadphase
  btScalar dx(bullet_sap_extents);
  btVector3 extents(dx, dx, dx);

  switch (bullet_broadphase_algorithm) {
    case BA_sweep_and_prune:
      _broadphase = new btAxisSweep3(extents, extents, 1024);
      break;
    case BA_dynamic_aabb_tree:
      _broadphase = new btDbvtBroadphase();
      break;
    default:
      bullet_cat.error() << "no proper broadphase algorithm!" << endl;
  }
  nassertv(_broadphase);

  // Configuration
  _configuration = new btSoftBodyRigidBodyCollisionConfiguration();
  nassertv(_configuration);

  // Dispatcher
  _dispatcher = new btCollisionDispatcher(_configuration);
  nassertv(_dispatcher);

  // Solver
  _solver = new btSequentialImpulseConstraintSolver;
  nassertv(_solver);

  // World
  _world = new btSoftRigidDynamicsWorld(_dispatcher, _broadphase, _solver, _configuration);
  nassertv(_world);
  nassertv(_world->getPairCache());

  _world->setWorldUserInfo(this);
  _world->setGravity(btVector3(0.0f, 0.0f, 0.0f));

  // Ghost-pair callback
  _world->getPairCache()->setInternalGhostPairCallback(&_ghost_cb);

  // Filter callback
  _filter_algorithm = bullet_filter_algorithm;
  switch (_filter_algorithm) {
    case FA_mask:
      _filter_cb = &_filter_cb1;
      break;
    case FA_groups_mask:
      _filter_cb = &_filter_cb2;
      break;
    case FA_callback:
      _filter_cb = &_filter_cb3;
      break;
    default:
      bullet_cat.error() << "no proper filter algorithm!" << endl;
      _filter_cb = nullptr;
  }

  _world->getPairCache()->setOverlapFilterCallback(_filter_cb);

  // Tick callback
  _tick_callback_obj = nullptr;

  // SoftBodyWorldInfo
  _info.m_dispatcher = _dispatcher;
  _info.m_broadphase = _broadphase;
  _info.m_gravity.setValue(0.0f, 0.0f, 0.0f);
  _info.m_sparsesdf.Initialize();

  // Register GIMPACT algorithm
  btGImpactCollisionAlgorithm::registerAlgorithm(_dispatcher);

  // Some prefered settings
  _world->getDispatchInfo().m_enableSPU = true;      // default: true
  _world->getDispatchInfo().m_useContinuous = true;  // default: true
  _world->getSolverInfo().m_splitImpulse = false;    // default: false
  _world->getSolverInfo().m_numIterations = bullet_solver_iterations;
}

/**
 *
 */
LightMutex &BulletWorld::
get_global_lock() {
  
  static LightMutex lock;
  
  return lock;
}

/**
 *
 */
BulletSoftBodyWorldInfo BulletWorld::
get_world_info() {

  return BulletSoftBodyWorldInfo(_info);
}

/**
 *
 */
void BulletWorld::
set_debug_node(BulletDebugNode *node) {
  LightMutexHolder holder(get_global_lock());

  nassertv(node);
  if (node != _debug) {
    if (_debug != nullptr) {
        _debug->_debug_stale = false;
        _debug->_debug_world = nullptr;
    }

    _debug = node;
    _world->setDebugDrawer(&(_debug->_drawer));
  }
}

/**
 * Removes a debug node that has been assigned to this BulletWorld.
 */
void BulletWorld::
clear_debug_node() {
  LightMutexHolder holder(get_global_lock());

  if (_debug != nullptr) {
    _debug->_debug_stale = false;
    _debug->_debug_world = nullptr;
    _world->setDebugDrawer(nullptr);
    _debug = nullptr;
  }
}

/**
 *
 */
void BulletWorld::
set_gravity(const LVector3 &gravity) {
  LightMutexHolder holder(get_global_lock());

  _world->setGravity(LVecBase3_to_btVector3(gravity));
  _info.m_gravity.setValue(gravity.get_x(), gravity.get_y(), gravity.get_z());
}

/**
 *
 */
void BulletWorld::
set_gravity(PN_stdfloat gx, PN_stdfloat gy, PN_stdfloat gz) {
  LightMutexHolder holder(get_global_lock());

  _world->setGravity(btVector3((btScalar)gx, (btScalar)gy, (btScalar)gz));
  _info.m_gravity.setValue((btScalar)gx, (btScalar)gy, (btScalar)gz);
}

/**
 *
 */
const LVector3 BulletWorld::
get_gravity() const {
  LightMutexHolder holder(get_global_lock());

  return btVector3_to_LVector3(_world->getGravity());
}

/**
 *
 */
int BulletWorld::
do_physics(PN_stdfloat dt, int max_substeps, PN_stdfloat stepsize) {
  LightMutexHolder holder(get_global_lock());

  bullet_contact_added_callback = _contact_added_callback_obj;

  _pstat_physics.start();

  int num_substeps = clamp(int(dt / stepsize), 1, max_substeps);

  // Synchronize Panda to Bullet
  _pstat_p2b.start();
  do_sync_p2b(dt, num_substeps);
  _pstat_p2b.stop();

  // Simulation
  _pstat_simulation.start();
  int n = _world->stepSimulation((btScalar)dt, max_substeps, (btScalar)stepsize);
  _pstat_simulation.stop();

  // Synchronize Bullet to Panda
  _pstat_b2p.start();
  do_sync_b2p();
  _info.m_sparsesdf.GarbageCollect(bullet_gc_lifetime);
  _pstat_b2p.stop();

  // Render debug
  if (_debug) {
    _debug->do_sync_b2p(_world);
  }

  _pstat_physics.stop();

  bullet_contact_added_callback.clear();

  return n;
}

/**
 * Assumes the lock(bullet global lock) is held by the caller
 */
void BulletWorld::
do_sync_p2b(PN_stdfloat dt, int num_substeps) {

  for (BulletRigidBodyNode *body : _bodies) {
    body->do_sync_p2b();
  }

  for (BulletSoftBodyNode *softbody : _softbodies) {
    softbody->do_sync_p2b();
  }

  for (BulletGhostNode *ghost : _ghosts) {
    ghost->do_sync_p2b();
  }

  for (BulletBaseCharacterControllerNode *character : _characters) {
    character->do_sync_p2b(dt, num_substeps);
  }
}

/**
 * Assumes the lock(bullet global lock) is held by the caller
 */
void BulletWorld::
do_sync_b2p() {

  for (BulletRigidBodyNode *body : _bodies) {
    body->do_sync_b2p();
  }

  for (BulletSoftBodyNode *softbody : _softbodies) {
    softbody->do_sync_b2p();
  }

  for (BulletGhostNode *ghost : _ghosts) {
    ghost->do_sync_b2p();
  }

  for (BulletBaseCharacterControllerNode *character : _characters) {
    character->do_sync_b2p();
  }

  for (BulletVehicle *vehicle : _vehicles) {
    vehicle->do_sync_b2p();
  }
}

/**
 *
 */
void BulletWorld::
attach(TypedObject *object) {
  LightMutexHolder holder(get_global_lock());

  if (object->is_of_type(BulletGhostNode::get_class_type())) {
    do_attach_ghost(DCAST(BulletGhostNode, object));
  }
  else if (object->is_of_type(BulletRigidBodyNode::get_class_type())) {
    do_attach_rigid_body(DCAST(BulletRigidBodyNode, object));
  }
  else if (object->is_of_type(BulletSoftBodyNode::get_class_type())) {
    do_attach_soft_body(DCAST(BulletSoftBodyNode, object));
  }
  else if (object->is_of_type(BulletBaseCharacterControllerNode::get_class_type())) {
    do_attach_character(DCAST(BulletBaseCharacterControllerNode, object));
  }
  else if (object->is_of_type(BulletVehicle::get_class_type())) {
    do_attach_vehicle(DCAST(BulletVehicle, object));
  }
  else if (object->is_of_type(BulletConstraint::get_class_type())) {
    do_attach_constraint(DCAST(BulletConstraint, object));
  }
  else {
    bullet_cat->error() << "not a bullet world object!" << endl;
  }
}

/**
 *
 */
void BulletWorld::
remove(TypedObject *object) {
  LightMutexHolder holder(get_global_lock());

  if (object->is_of_type(BulletGhostNode::get_class_type())) {
    do_remove_ghost(DCAST(BulletGhostNode, object));
  }
  else if (object->is_of_type(BulletRigidBodyNode::get_class_type())) {
    do_remove_rigid_body(DCAST(BulletRigidBodyNode, object));
  }
  else if (object->is_of_type(BulletSoftBodyNode::get_class_type())) {
    do_remove_soft_body(DCAST(BulletSoftBodyNode, object));
  }
  else if (object->is_of_type(BulletBaseCharacterControllerNode::get_class_type())) {
    do_remove_character(DCAST(BulletBaseCharacterControllerNode, object));
  }
  else if (object->is_of_type(BulletVehicle::get_class_type())) {
    do_remove_vehicle(DCAST(BulletVehicle, object));
  }
  else if (object->is_of_type(BulletConstraint::get_class_type())) {
    do_remove_constraint(DCAST(BulletConstraint, object));
  }
  else {
    bullet_cat->error() << "not a bullet world object!" << endl;
  }
}

/**
 * @deprecated Please use BulletWorld::attach
 */
void BulletWorld::
attach_rigid_body(BulletRigidBodyNode *node) {
  LightMutexHolder holder(get_global_lock());

  do_attach_rigid_body(node);
}

/**
 * @deprecated Please use BulletWorld::remove
 */
void BulletWorld::
remove_rigid_body(BulletRigidBodyNode *node) {
  LightMutexHolder holder(get_global_lock());

  do_remove_rigid_body(node);
}

/**
 * @deprecated Please use BulletWorld::attach
 */
void BulletWorld::
attach_soft_body(BulletSoftBodyNode *node) {
  LightMutexHolder holder(get_global_lock());

  do_attach_soft_body(node);
}

/**
 * @deprecated Please use BulletWorld::remove
 */
void BulletWorld::
remove_soft_body(BulletSoftBodyNode *node) {
  LightMutexHolder holder(get_global_lock());

  do_remove_soft_body(node);
}

/**
 * @deprecated Please use BulletWorld::attach
 */
void BulletWorld::
attach_ghost(BulletGhostNode *node) {
  LightMutexHolder holder(get_global_lock());

  do_attach_ghost(node);
}

/**
 * @deprecated Please use BulletWorld::remove
 */
void BulletWorld::
remove_ghost(BulletGhostNode *node) {
  LightMutexHolder holder(get_global_lock());

  do_remove_ghost(node);
}

/**
 * @deprecated Please use BulletWorld::attach
 */
void BulletWorld::
attach_character(BulletBaseCharacterControllerNode *node) {
  LightMutexHolder holder(get_global_lock());

  do_attach_character(node);
}

/**
 * @deprecated Please use BulletWorld::remove
 */
void BulletWorld::
remove_character(BulletBaseCharacterControllerNode *node) {
  LightMutexHolder holder(get_global_lock());

  do_remove_character(node);
}

/**
 * @deprecated Please use BulletWorld::attach
 */
void BulletWorld::
attach_vehicle(BulletVehicle *vehicle) {
  LightMutexHolder holder(get_global_lock());

  do_attach_vehicle(vehicle);
}

/**
 * @deprecated Please use BulletWorld::remove
 */
void BulletWorld::
remove_vehicle(BulletVehicle *vehicle) {
  LightMutexHolder holder(get_global_lock());

  do_remove_vehicle(vehicle);
}

/**
 * Attaches a single constraint to a world.  Collision checks between the
 * linked objects will be disabled if the second parameter is set to TRUE.
 */
void BulletWorld::
attach_constraint(BulletConstraint *constraint, bool linked_collision) {
  LightMutexHolder holder(get_global_lock());

  do_attach_constraint(constraint, linked_collision);
}

/**
 * @deprecated Please use BulletWorld::remove
 */
void BulletWorld::
remove_constraint(BulletConstraint *constraint) {
  LightMutexHolder holder(get_global_lock());

  do_remove_constraint(constraint);
}

/**
 * Assumes the lock(bullet global lock) is held by the caller
 */
void BulletWorld::
do_attach_rigid_body(BulletRigidBodyNode *node) {

  nassertv(node);

  btRigidBody *ptr = btRigidBody::upcast(node->get_object());

  BulletRigidBodies::iterator found;
  PT(BulletRigidBodyNode) ptnode = node;
  found = find(_bodies.begin(), _bodies.end(), ptnode);

  if (found == _bodies.end()) {
    _bodies.push_back(node);
    _world->addRigidBody(ptr);
  }
  else {
    bullet_cat.warning() << "rigid body already attached" << endl;
  }
}

/**
 * Assumes the lock(bullet global lock) is held by the caller
 */
void BulletWorld::
do_remove_rigid_body(BulletRigidBodyNode *node) {

  nassertv(node);

  btRigidBody *ptr = btRigidBody::upcast(node->get_object());

  BulletRigidBodies::iterator found;
  PT(BulletRigidBodyNode) ptnode = node;
  found = find(_bodies.begin(), _bodies.end(), ptnode);

  if (found == _bodies.end()) {
    bullet_cat.warning() << "rigid body not attached" << endl;
  }
  else {
    _bodies.erase(found);
    _world->removeRigidBody(ptr);
  }
}

/**
 * Assumes the lock(bullet global lock) is held by the caller
 */
void BulletWorld::
do_attach_soft_body(BulletSoftBodyNode *node) {

  nassertv(node);

  btSoftBody *ptr = btSoftBody::upcast(node->get_object());

  // TODO: groupfilter settings (see ghost objects too)
  short group = btBroadphaseProxy::DefaultFilter;
  short mask = btBroadphaseProxy::AllFilter;

  BulletSoftBodies::iterator found;
  PT(BulletSoftBodyNode) ptnode = node;
  found = find(_softbodies.begin(), _softbodies.end(), ptnode);

  if (found == _softbodies.end()) {
    _softbodies.push_back(node);
    _world->addSoftBody(ptr, group, mask);
  }
  else {
    bullet_cat.warning() << "soft body already attached" << endl;
  }
}

/**
 * Assumes the lock(bullet global lock) is held by the caller
 */
void BulletWorld::
do_remove_soft_body(BulletSoftBodyNode *node) {

  nassertv(node);

  btSoftBody *ptr = btSoftBody::upcast(node->get_object());

  BulletSoftBodies::iterator found;
  PT(BulletSoftBodyNode) ptnode = node;
  found = find(_softbodies.begin(), _softbodies.end(), ptnode);

  if (found == _softbodies.end()) {
    bullet_cat.warning() << "soft body not attached" << endl;
  }
  else {
    _softbodies.erase(found);
    _world->removeSoftBody(ptr);
  }
}

/**
 * Assumes the lock(bullet global lock) is held by the caller
 */
void BulletWorld::
do_attach_ghost(BulletGhostNode *node) {

  nassertv(node);

  // TODO groupfilter settings...
/*
enum CollisionFilterGroups {
  DefaultFilter = 1,
  StaticFilter = 2,
  KinematicFilter = 4,
  DebrisFilter = 8,
  SensorTrigger = 16,
  CharacterFilter = 32,
  AllFilter = -1
}
*/

  short group = btBroadphaseProxy::SensorTrigger;
  short mask = btBroadphaseProxy::AllFilter
            & ~btBroadphaseProxy::StaticFilter
            & ~btBroadphaseProxy::SensorTrigger;

  btGhostObject *ptr = btGhostObject::upcast(node->get_object());

  BulletGhosts::iterator found;
  PT(BulletGhostNode) ptnode = node;
  found = find(_ghosts.begin(), _ghosts.end(), ptnode);

  if (found == _ghosts.end()) {
    _ghosts.push_back(node);
    _world->addCollisionObject(ptr, group, mask);
  }
  else {
    bullet_cat.warning() << "ghost already attached" << endl;
  }
}

/**
 * Assumes the lock(bullet global lock) is held by the caller
 */
void BulletWorld::
do_remove_ghost(BulletGhostNode *node) {

  nassertv(node);

  btGhostObject *ptr = btGhostObject::upcast(node->get_object());

  BulletGhosts::iterator found;
  PT(BulletGhostNode) ptnode = node;
  found = find(_ghosts.begin(), _ghosts.end(), ptnode);

  if (found == _ghosts.end()) {
    bullet_cat.warning() << "ghost not attached" << endl;
  }
  else {
    _ghosts.erase(found);
    _world->removeCollisionObject(ptr);
  }
}

/**
 * Assumes the lock(bullet global lock) is held by the caller
 */
void BulletWorld::
do_attach_character(BulletBaseCharacterControllerNode *node) {

  nassertv(node);

  BulletCharacterControllers::iterator found;
  PT(BulletBaseCharacterControllerNode) ptnode = node;
  found = find(_characters.begin(), _characters.end(), ptnode);

  if (found == _characters.end()) {
    _characters.push_back(node);

    _world->addCollisionObject(node->get_ghost(),
      btBroadphaseProxy::CharacterFilter,
      btBroadphaseProxy::StaticFilter|btBroadphaseProxy::DefaultFilter);

    _world->addCharacter(node->get_character());
  }
  else {
    bullet_cat.warning() << "character already attached" << endl;
  }
}

/**
 * Assumes the lock(bullet global lock) is held by the caller
 */
void BulletWorld::
do_remove_character(BulletBaseCharacterControllerNode *node) {

  nassertv(node);

  BulletCharacterControllers::iterator found;
  PT(BulletBaseCharacterControllerNode) ptnode = node;
  found = find(_characters.begin(), _characters.end(), ptnode);

  if (found == _characters.end()) {
    bullet_cat.warning() << "character not attached" << endl;
  }
  else {
    _characters.erase(found);
    _world->removeCollisionObject(node->get_ghost());
    _world->removeCharacter(node->get_character());
  }
}

/**
 * Assumes the lock(bullet global lock) is held by the caller
 */
void BulletWorld::
do_attach_vehicle(BulletVehicle *vehicle) {

  nassertv(vehicle);

  BulletVehicles::iterator found;
  PT(BulletVehicle) ptvehicle = vehicle;
  found = find(_vehicles.begin(), _vehicles.end(), ptvehicle);

  if (found == _vehicles.end()) {
    _vehicles.push_back(vehicle);
    _world->addVehicle(vehicle->get_vehicle());
  }
  else {
    bullet_cat.warning() << "vehicle already attached" << endl;
  }
}

/**
 * Assumes the lock(bullet global lock) is held by the caller
 */
void BulletWorld::
do_remove_vehicle(BulletVehicle *vehicle) {

  nassertv(vehicle);

  do_remove_rigid_body(vehicle->do_get_chassis());

  BulletVehicles::iterator found;
  PT(BulletVehicle) ptvehicle = vehicle;
  found = find(_vehicles.begin(), _vehicles.end(), ptvehicle);

  if (found == _vehicles.end()) {
    bullet_cat.warning() << "vehicle not attached" << endl;
  }
  else {
    _vehicles.erase(found);
    _world->removeVehicle(vehicle->get_vehicle());
  }
}

/**
 * Attaches a single constraint to a world.  Collision checks between the
 * linked objects will be disabled if the second parameter is set to TRUE.
 * Assumes the lock(bullet global lock) is held by the caller
 */
void BulletWorld::
do_attach_constraint(BulletConstraint *constraint, bool linked_collision) {

  nassertv(constraint);

  BulletConstraints::iterator found;
  PT(BulletConstraint) ptconstraint = constraint;
  found = find(_constraints.begin(), _constraints.end(), ptconstraint);

  if (found == _constraints.end()) {
    _constraints.push_back(constraint);
    _world->addConstraint(constraint->ptr(), linked_collision);
  }
  else {
    bullet_cat.warning() << "constraint already attached" << endl;
  }
}

/**
 * Assumes the lock(bullet global lock) is held by the caller
 */
void BulletWorld::
do_remove_constraint(BulletConstraint *constraint) {

  nassertv(constraint);

  BulletConstraints::iterator found;
  PT(BulletConstraint) ptconstraint = constraint;
  found = find(_constraints.begin(), _constraints.end(), ptconstraint);

  if (found == _constraints.end()) {
    bullet_cat.warning() << "constraint not attached" << endl;
  }
  else {
    _constraints.erase(found);
    _world->removeConstraint(constraint->ptr());
  }
}

/**
 *
 */
int BulletWorld::
get_num_rigid_bodies() const {
  LightMutexHolder holder(get_global_lock());

  return _bodies.size();
}

/**
 *
 */
BulletRigidBodyNode *BulletWorld::
get_rigid_body(int idx) const {
  LightMutexHolder holder(get_global_lock());

  nassertr(idx >= 0 && idx < (int)_bodies.size(), nullptr);
  return _bodies[idx];
}

/**
 *
 */
int BulletWorld::
get_num_soft_bodies() const {
  LightMutexHolder holder(get_global_lock());

  return _softbodies.size();
}

/**
 *
 */
BulletSoftBodyNode *BulletWorld::
get_soft_body(int idx) const {
  LightMutexHolder holder(get_global_lock());

  nassertr(idx >= 0 && idx < (int)_softbodies.size(), nullptr);
  return _softbodies[idx];
}

/**
 *
 */
int BulletWorld::
get_num_ghosts() const {
  LightMutexHolder holder(get_global_lock());

  return _ghosts.size();
}

/**
 *
 */
BulletGhostNode *BulletWorld::
get_ghost(int idx) const {
  LightMutexHolder holder(get_global_lock());

  nassertr(idx >= 0 && idx < (int)_ghosts.size(), nullptr);
  return _ghosts[idx];
}

/**
 *
 */
int BulletWorld::
get_num_characters() const {
  LightMutexHolder holder(get_global_lock());

  return _characters.size();
}

/**
 *
 */
BulletBaseCharacterControllerNode *BulletWorld::
get_character(int idx) const {
  LightMutexHolder holder(get_global_lock());

  nassertr(idx >= 0 && idx < (int)_characters.size(), nullptr);
  return _characters[idx];
}

/**
 *
 */
int BulletWorld::
get_num_vehicles() const {
  LightMutexHolder holder(get_global_lock());

  return _vehicles.size();
}

/**
 *
 */
BulletVehicle *BulletWorld::
get_vehicle(int idx) const {
  LightMutexHolder holder(get_global_lock());

  nassertr(idx >= 0 && idx < (int)_vehicles.size(), nullptr);
  return _vehicles[idx];
}

/**
 *
 */
int BulletWorld::
get_num_constraints() const {
  LightMutexHolder holder(get_global_lock());

  return _constraints.size();
}

/**
 *
 */
BulletConstraint *BulletWorld::
get_constraint(int idx) const {
  LightMutexHolder holder(get_global_lock());

  nassertr(idx >= 0 && idx < (int)_constraints.size(), nullptr);
  return _constraints[idx];
}

/**
 *
 */
int BulletWorld::
get_num_manifolds() const {
  LightMutexHolder holder(get_global_lock());

  return _world->getDispatcher()->getNumManifolds();
}

/**
 *
 */
BulletClosestHitRayResult BulletWorld::
ray_test_closest(const LPoint3 &from_pos, const LPoint3 &to_pos, const CollideMask &mask) const {
  LightMutexHolder holder(get_global_lock());

  nassertr(!from_pos.is_nan(), BulletClosestHitRayResult::empty());
  nassertr(!to_pos.is_nan(), BulletClosestHitRayResult::empty());

  const btVector3 from = LVecBase3_to_btVector3(from_pos);
  const btVector3 to = LVecBase3_to_btVector3(to_pos);

  BulletClosestHitRayResult cb(from, to, mask);
  _world->rayTest(from, to, cb);
  return cb;
}

/**
 *
 */
BulletAllHitsRayResult BulletWorld::
ray_test_all(const LPoint3 &from_pos, const LPoint3 &to_pos, const CollideMask &mask) const {
  LightMutexHolder holder(get_global_lock());

  nassertr(!from_pos.is_nan(), BulletAllHitsRayResult::empty());
  nassertr(!to_pos.is_nan(), BulletAllHitsRayResult::empty());

  const btVector3 from = LVecBase3_to_btVector3(from_pos);
  const btVector3 to = LVecBase3_to_btVector3(to_pos);

  BulletAllHitsRayResult cb(from, to, mask);
  _world->rayTest(from, to, cb);
  return cb;
}

/**
 * Performs a sweep test against all other shapes that match the given group
 * mask.  The provided shape must be a convex shape; it is an error to invoke
 * this method using a non-convex shape.
 */
BulletClosestHitSweepResult BulletWorld::
sweep_test_closest(BulletShape *shape, const TransformState &from_ts, const TransformState &to_ts, const CollideMask &mask, PN_stdfloat penetration) const {
  LightMutexHolder holder(get_global_lock());

  nassertr(shape, BulletClosestHitSweepResult::empty());

  const btConvexShape *convex = (const btConvexShape *) shape->ptr();
  nassertr(convex->isConvex(), BulletClosestHitSweepResult::empty());
  
  nassertr(!from_ts.is_invalid(), BulletClosestHitSweepResult::empty());
  nassertr(!to_ts.is_invalid(), BulletClosestHitSweepResult::empty());

  const btVector3 from_pos = LVecBase3_to_btVector3(from_ts.get_pos());
  const btVector3 to_pos = LVecBase3_to_btVector3(to_ts.get_pos());
  const btTransform from_trans = LMatrix4_to_btTrans(from_ts.get_mat());
  const btTransform to_trans = LMatrix4_to_btTrans(to_ts.get_mat());

  BulletClosestHitSweepResult cb(from_pos, to_pos, mask);
  _world->convexSweepTest(convex, from_trans, to_trans, cb, penetration);
  return cb;
}

/**
 * Performs a test if two bodies should collide or not, based on the collision
 * filter setting.
 */
bool BulletWorld::
filter_test(PandaNode *node0, PandaNode *node1) const {
  LightMutexHolder holder(get_global_lock());

  nassertr(node0, false);
  nassertr(node1, false);
  nassertr(_filter_cb, false);

  btCollisionObject *obj0 = get_collision_object(node0);
  btCollisionObject *obj1 = get_collision_object(node1);

  nassertr(obj0, false);
  nassertr(obj1, false);

  btBroadphaseProxy *proxy0 = obj0->getBroadphaseHandle();
  btBroadphaseProxy *proxy1 = obj1->getBroadphaseHandle();

  nassertr(proxy0, false);
  nassertr(proxy1, false);

  return _filter_cb->needBroadphaseCollision(proxy0, proxy1);
}

/**
 * Performas a test for all bodies which are currently in contact with the
 * given body.  The test returns a BulletContactResult object which may
 * contain zero, one or more contacts.
 *
 * If the optional parameter use_filter is set to TRUE this test will consider
 * filter settings.  Otherwise all objects in contact are reported, no matter
 * if they would collide or not.
 */
BulletContactResult BulletWorld::
contact_test(PandaNode *node, bool use_filter) const {
  LightMutexHolder holder(get_global_lock());

  btCollisionObject *obj = get_collision_object(node);

  BulletContactResult cb;

  if (obj) {
#if BT_BULLET_VERSION >= 281
    if (use_filter) {
      cb.use_filter(_filter_cb, obj->getBroadphaseHandle());
    }
#endif

    _world->contactTest(obj, cb);
  }

  return cb;
}

/**
 * Performas a test if the two bodies given as parameters are in contact or
 * not.  The test returns a BulletContactResult object which may contain zero
 * or one contacts.
 */
BulletContactResult BulletWorld::
contact_test_pair(PandaNode *node0, PandaNode *node1) const {
  LightMutexHolder holder(get_global_lock());

  btCollisionObject *obj0 = get_collision_object(node0);
  btCollisionObject *obj1 = get_collision_object(node1);

  BulletContactResult cb;

  if (obj0 && obj1) {

    _world->contactPairTest(obj0, obj1, cb);
  }

  return cb;
}

/**
 *
 */
BulletPersistentManifold *BulletWorld::
get_manifold(int idx) const {
  LightMutexHolder holder(get_global_lock());

  nassertr(idx < _dispatcher->getNumManifolds(), nullptr);

  btPersistentManifold *ptr = _dispatcher->getManifoldByIndexInternal(idx);
  return (ptr) ? new BulletPersistentManifold(ptr) : nullptr;
}

/**
 *
 */
btCollisionObject *BulletWorld::
get_collision_object(PandaNode *node) {

  if (node->is_of_type(BulletRigidBodyNode::get_class_type())) {
    return ((BulletRigidBodyNode *)node)->get_object();
  }
  else if (node->is_of_type(BulletGhostNode::get_class_type())) {
    return ((BulletGhostNode *)node)->get_object();
  }
  else if (node->is_of_type(BulletBaseCharacterControllerNode::get_class_type())) {
    return ((BulletBaseCharacterControllerNode *)node)->get_ghost();
  }
  else if (node->is_of_type(BulletSoftBodyNode::get_class_type())) {
    return ((BulletSoftBodyNode *)node)->get_object();
  }

  return nullptr;
}

/**
 *
 */
void BulletWorld::
set_group_collision_flag(unsigned int group1, unsigned int group2, bool enable) {
  LightMutexHolder holder(get_global_lock());

  if (_filter_algorithm != FA_groups_mask) {
    bullet_cat.warning() << "filter algorithm is not 'groups-mask'" << endl;
  }

  _filter_cb2._collide[group1].set_bit_to(group2, enable);
  _filter_cb2._collide[group2].set_bit_to(group1, enable);
}

/**
 *
 */
bool BulletWorld::
get_group_collision_flag(unsigned int group1, unsigned int group2) const {
  LightMutexHolder holder(get_global_lock());

  return _filter_cb2._collide[group1].get_bit(group2);
}

/**
 *
 */
void BulletWorld::
set_force_update_all_aabbs(bool force) {
  LightMutexHolder holder(get_global_lock());
  _world->setForceUpdateAllAabbs(force);
}

/**
 *
 */
bool BulletWorld::
get_force_update_all_aabbs() const {
  LightMutexHolder holder(get_global_lock());
  return _world->getForceUpdateAllAabbs();
}

/**
 *
 */
void BulletWorld::
set_contact_added_callback(CallbackObject *obj) {
  LightMutexHolder holder(get_global_lock());

  _world->getSolverInfo().m_solverMode |= SOLVER_DISABLE_VELOCITY_DEPENDENT_FRICTION_DIRECTION;
  _world->getSolverInfo().m_solverMode |= SOLVER_USE_2_FRICTION_DIRECTIONS;
  _world->getSolverInfo().m_solverMode |= SOLVER_ENABLE_FRICTION_DIRECTION_CACHING;

  _contact_added_callback_obj = obj;
}

/**
 *
 */
void BulletWorld::
clear_contact_added_callback() {
  LightMutexHolder holder(get_global_lock());

  _world->getSolverInfo().m_solverMode &= ~SOLVER_DISABLE_VELOCITY_DEPENDENT_FRICTION_DIRECTION;
  _world->getSolverInfo().m_solverMode &= ~SOLVER_USE_2_FRICTION_DIRECTIONS;
  _world->getSolverInfo().m_solverMode &= ~SOLVER_ENABLE_FRICTION_DIRECTION_CACHING;

  _contact_added_callback_obj = nullptr;
}

/**
 *
 */
void BulletWorld::
set_tick_callback(CallbackObject *obj, bool is_pretick) {
  LightMutexHolder holder(get_global_lock());

  nassertv(obj != nullptr);
  _tick_callback_obj = obj;
  _world->setInternalTickCallback(&BulletWorld::tick_callback, this, is_pretick);
}

/**
 *
 */
void BulletWorld::
clear_tick_callback() {
  LightMutexHolder holder(get_global_lock());

  _tick_callback_obj = nullptr;
  _world->setInternalTickCallback(nullptr);
}

/**
 *
 */
void BulletWorld::
tick_callback(btDynamicsWorld *world, btScalar timestep) {

  nassertv(world->getWorldUserInfo());

  BulletWorld *w = static_cast<BulletWorld *>(world->getWorldUserInfo());
  CallbackObject *obj = w->_tick_callback_obj;
  if (obj) {
    BulletTickCallbackData cbdata(timestep);
    // Release the global lock that we are holding during the tick callback
    // and allow interactions with bullet world in the user callback
    get_global_lock().release();
    obj->do_callback(&cbdata);
    // Acquire the global lock again and protect the execution
    get_global_lock().acquire();
  }
}

/**
 *
 */
void BulletWorld::
set_filter_callback(CallbackObject *obj) {
  LightMutexHolder holder(get_global_lock());

  nassertv(obj != nullptr);

  if (_filter_algorithm != FA_callback) {
    bullet_cat.warning() << "filter algorithm is not 'callback'" << endl;
  }

  _filter_cb3._filter_callback_obj = obj;
}

/**
 *
 */
void BulletWorld::
clear_filter_callback() {
  LightMutexHolder holder(get_global_lock());

  _filter_cb3._filter_callback_obj = nullptr;
}

/**
 *
 */
bool BulletWorld::btFilterCallback1::
needBroadphaseCollision(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1) const {

  btCollisionObject *obj0 = (btCollisionObject *) proxy0->m_clientObject;
  btCollisionObject *obj1 = (btCollisionObject *) proxy1->m_clientObject;

  nassertr(obj0, false);
  nassertr(obj1, false);

  PandaNode *node0 = (PandaNode *) obj0->getUserPointer();
  PandaNode *node1 = (PandaNode *) obj1->getUserPointer();

  nassertr(node0, false);
  nassertr(node1, false);

  CollideMask mask0 = node0->get_into_collide_mask();
  CollideMask mask1 = node1->get_into_collide_mask();

  return (mask0 & mask1) != 0;
}

/**
 *
 */
bool BulletWorld::btFilterCallback2::
needBroadphaseCollision(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1) const {

  btCollisionObject *obj0 = (btCollisionObject *) proxy0->m_clientObject;
  btCollisionObject *obj1 = (btCollisionObject *) proxy1->m_clientObject;

  nassertr(obj0, false);
  nassertr(obj1, false);

  PandaNode *node0 = (PandaNode *) obj0->getUserPointer();
  PandaNode *node1 = (PandaNode *) obj1->getUserPointer();

  nassertr(node0, false);
  nassertr(node1, false);

  CollideMask mask0 = node0->get_into_collide_mask();
  CollideMask mask1 = node1->get_into_collide_mask();

// cout << mask0 << "   " << mask1 << endl;

  for (size_t i = 0; i < 32; ++i) {
    if (mask0.get_bit(i)) {
      if ((_collide[i] & mask1) != 0)
// cout << "collide: i=" << i << " _collide[i]" << _collide[i] << endl;
        return true;
    }
  }

  return false;
}

/**
 *
 */
bool BulletWorld::btFilterCallback3::
needBroadphaseCollision(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1) const {

  nassertr(_filter_callback_obj, false);

  btCollisionObject *obj0 = (btCollisionObject *) proxy0->m_clientObject;
  btCollisionObject *obj1 = (btCollisionObject *) proxy1->m_clientObject;

  nassertr(obj0, false);
  nassertr(obj1, false);

  PandaNode *node0 = (PandaNode *) obj0->getUserPointer();
  PandaNode *node1 = (PandaNode *) obj1->getUserPointer();

  nassertr(node0, false);
  nassertr(node1, false);

  BulletFilterCallbackData cbdata(node0, node1);
  _filter_callback_obj->do_callback(&cbdata);
  return cbdata.get_collide();
}

/**
 *
 */
ostream &
operator << (ostream &out, BulletWorld::BroadphaseAlgorithm algorithm) {

  switch (algorithm) {
  case BulletWorld::BA_sweep_and_prune:
    return out << "sap";

  case BulletWorld::BA_dynamic_aabb_tree:
    return out << "aabb";
  };

  return out << "**invalid BulletWorld::BroadphaseAlgorithm(" << (int)algorithm << ")**";
}

/**
 *
 */
istream &
operator >> (istream &in, BulletWorld::BroadphaseAlgorithm &algorithm) {
  string word;
  in >> word;

  if (word == "sap") {
    algorithm = BulletWorld::BA_sweep_and_prune;
  }
  else if (word == "aabb") {
    algorithm = BulletWorld::BA_dynamic_aabb_tree;
  }
  else {
    bullet_cat.error()
      << "Invalid BulletWorld::BroadphaseAlgorithm: " << word << "\n";
    algorithm = BulletWorld::BA_dynamic_aabb_tree;
  }

  return in;
}

/**
 *
 */
ostream &
operator << (ostream &out, BulletWorld::FilterAlgorithm algorithm) {

  switch (algorithm) {
  case BulletWorld::FA_mask:
    return out << "mask";
  case BulletWorld::FA_groups_mask:
    return out << "groups-mask";
  case BulletWorld::FA_callback:
    return out << "callback";
  };
  return out << "**invalid BulletWorld::FilterAlgorithm(" << (int)algorithm << ")**";
}

/**
 *
 */
istream &
operator >> (istream &in, BulletWorld::FilterAlgorithm &algorithm) {
  string word;
  in >> word;

  if (word == "mask") {
    algorithm = BulletWorld::FA_mask;
  }
  else if (word == "groups-mask") {
    algorithm = BulletWorld::FA_groups_mask;
  }
  else if (word == "callback") {
    algorithm = BulletWorld::FA_callback;
  }
  else {
    bullet_cat.error()
      << "Invalid BulletWorld::FilterAlgorithm: " << word << "\n";
    algorithm = BulletWorld::FA_mask;
  }
  return in;
}
