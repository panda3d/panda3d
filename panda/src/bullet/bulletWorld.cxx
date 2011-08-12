// Filename: bulletWorld.cxx
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

#include "bulletWorld.h"
#include "bulletPersistentManifold.h"
#include "bulletShape.h"
#include "bulletSoftBodyWorldInfo.h"

#include "collideMask.h"

TypeHandle BulletWorld::_type_handle;

PStatCollector BulletWorld::_pstat_physics("App:Bullet:DoPhysics");
PStatCollector BulletWorld::_pstat_simulation("App:Bullet:DoPhysics:Simulation");
PStatCollector BulletWorld::_pstat_debug("App:Bullet:DoPhysics:Debug");
PStatCollector BulletWorld::_pstat_p2b("App:Bullet:DoPhysics:SyncP2B");
PStatCollector BulletWorld::_pstat_b2p("App:Bullet:DoPhysics:SyncB2P");

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletWorld::
BulletWorld() {

  // Callbacks
  _ghost_cb = new btGhostPairCallback();
  _filter_cb = new btFilterCallback();

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

  // Configuration
  _configuration = new btSoftBodyRigidBodyCollisionConfiguration();

  // Dispatcher
  _dispatcher = new btCollisionDispatcher(_configuration);

  // Solver
  _solver = new btSequentialImpulseConstraintSolver;

  // World
  _world = new btSoftRigidDynamicsWorld(_dispatcher, _broadphase, _solver, _configuration);
  _world->getPairCache()->setInternalGhostPairCallback(_ghost_cb);
  _world->getPairCache()->setOverlapFilterCallback(_filter_cb);
  _world->setGravity(btVector3(0.0f, 0.0f, 0.0f));

  // SoftBodyWorldInfo
  _info.m_dispatcher = _dispatcher;
  _info.m_broadphase = _broadphase;
  _info.m_gravity = _world->getGravity();
  _info.m_sparsesdf.Initialize();

  // Register GIMPACT algorithm
  btGImpactCollisionAlgorithm::registerAlgorithm(_dispatcher);

  // Some prefered settings
  _world->getDispatchInfo().m_enableSPU = true;      // default: true
  _world->getDispatchInfo().m_useContinuous = true;  // default: true
  _world->getSolverInfo().m_splitImpulse = false;    // default: false
  _world->getSolverInfo().m_numIterations = bullet_solver_iterations;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::get_world_info
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletSoftBodyWorldInfo BulletWorld::
get_world_info() {

  return BulletSoftBodyWorldInfo(_info);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::set_gravity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletWorld::
set_gravity(const LVector3f &gravity) {

  _world->setGravity(LVecBase3f_to_btVector3(gravity));
  _info.m_gravity = _world->getGravity();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::set_gravity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletWorld::
set_gravity(float gx, float gy, float gz) {

  _world->setGravity(btVector3(gx, gy, gz));
  _info.m_gravity = _world->getGravity();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::get_gravity
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
const LVector3f BulletWorld::
get_gravity() const {

  return btVector3_to_LVector3f(_world->getGravity());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::do_physics
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletWorld::
do_physics(float dt, int substeps, float stepsize) {

  _pstat_physics.start();

  // Synchronize Panda to Bullet
  _pstat_p2b.start();
  sync_p2b(dt);
  _pstat_p2b.stop();

  // Simulation
  _pstat_simulation.start();
  _world->stepSimulation(dt, substeps, stepsize);
  _pstat_simulation.stop();

  // Synchronize Bullet to Panda
  _pstat_b2p.start();
  sync_b2p();
  _info.m_sparsesdf.GarbageCollect(bullet_gc_lifetime);
  _pstat_b2p.stop();

  // Render debug
  if (_debug) {
    _pstat_debug.start();
    _debug->sync_b2p(_world);
    _pstat_debug.stop();
  }

  _pstat_physics.stop();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::sync_p2b
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void BulletWorld::
sync_p2b(float dt) {

  for (int i=0; i < get_num_rigid_bodies(); i++) {
    get_rigid_body(i)->sync_p2b();
  }

  for (int i=0; i < get_num_soft_bodies(); i++) {
    get_soft_body(i)->sync_p2b();
  }

  for (int i=0; i < get_num_ghosts(); i++) {
    get_ghost(i)->sync_p2b();
  }

  for (int i=0; i < get_num_characters(); i++) {
    get_character(i)->sync_p2b(dt);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::sync_b2p
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void BulletWorld::
sync_b2p() {

  for (int i=0; i < get_num_vehicles(); i++) {
    get_vehicle(i)->sync_b2p();
  }

  for (int i=0; i < get_num_rigid_bodies(); i++) {
    get_rigid_body(i)->sync_b2p();
  }

  for (int i=0; i < get_num_soft_bodies(); i++) {
    get_soft_body(i)->sync_b2p();
  }

  for (int i=0; i < get_num_ghosts(); i++) {
    get_ghost(i)->sync_b2p();
  }

  for (int i=0; i < get_num_characters(); i++) {
    get_character(i)->sync_b2p();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::set_debug_node
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletWorld::
set_debug_node(BulletDebugNode *node) {

  nassertv(node);

  _debug = node;
  _world->setDebugDrawer(&(_debug->_drawer));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::clear_debug_node
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletWorld::
clear_debug_node() {

  _debug = NULL;
  _world->setDebugDrawer(NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::attach_rigid_body
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletWorld::
attach_rigid_body(BulletRigidBodyNode *node) {

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

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::remove_rigid_body
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletWorld::
remove_rigid_body(BulletRigidBodyNode *node) {

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

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::attach_soft_body
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletWorld::
attach_soft_body(BulletSoftBodyNode *node) {

  nassertv(node);

  btSoftBody *ptr = btSoftBody::upcast(node->get_object());

  // TODO: group/filter settings (see ghost objects too)
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

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::remove_soft_body
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletWorld::
remove_soft_body(BulletSoftBodyNode *node) {

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

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::attach_ghost
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletWorld::
attach_ghost(BulletGhostNode *node) {

  nassertv(node);

  // TODO group/filter settings...
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

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::remove_ghost
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletWorld::
remove_ghost(BulletGhostNode *node) {

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

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::attach_character
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletWorld::
attach_character(BulletCharacterControllerNode *node) {

  nassertv(node);

  BulletCharacterControllers::iterator found;
  PT(BulletCharacterControllerNode) ptnode = node;
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

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::remove_character
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletWorld::
remove_character(BulletCharacterControllerNode *node) {

  nassertv(node);

  BulletCharacterControllers::iterator found;
  PT(BulletCharacterControllerNode) ptnode = node;
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

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::attach_vehicle
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletWorld::
attach_vehicle(BulletVehicle *vehicle) {

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

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::remove_vehicle
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletWorld::
remove_vehicle(BulletVehicle *vehicle) {

  nassertv(vehicle);

  remove_rigid_body(vehicle->get_chassis());

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

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::attach_constraint
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletWorld::
attach_constraint(BulletConstraint *constraint) {

  nassertv(constraint);

  BulletConstraints::iterator found;
  PT(BulletConstraint) ptconstraint = constraint;
  found = find(_constraints.begin(), _constraints.end(), ptconstraint);

  if (found == _constraints.end()) {
    _constraints.push_back(constraint);
    _world->addConstraint(constraint->ptr());
  }
  else {
    bullet_cat.warning() << "constraint already attached" << endl;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::remove_constraint
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletWorld::
remove_constraint(BulletConstraint *constraint) {

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

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::ray_test_closest
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletClosestHitRayResult BulletWorld::
ray_test_closest(const LPoint3f &from_pos, const LPoint3f &to_pos, const CollideMask &mask) const {

  nassertr(!from_pos.is_nan(), BulletClosestHitRayResult::empty());
  nassertr(!to_pos.is_nan(), BulletClosestHitRayResult::empty());

  const btVector3 from = LVecBase3f_to_btVector3(from_pos);
  const btVector3 to = LVecBase3f_to_btVector3(to_pos);

  BulletClosestHitRayResult cb(from, to, mask);
  _world->rayTest(from, to, cb);
  return cb;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::ray_test_all
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletAllHitsRayResult BulletWorld::
ray_test_all(const LPoint3f &from_pos, const LPoint3f &to_pos, const CollideMask &mask) const {

  nassertr(!from_pos.is_nan(), BulletAllHitsRayResult::empty());
  nassertr(!to_pos.is_nan(), BulletAllHitsRayResult::empty());

  const btVector3 from = LVecBase3f_to_btVector3(from_pos);
  const btVector3 to = LVecBase3f_to_btVector3(to_pos);

  BulletAllHitsRayResult cb(from, to, mask);
  _world->rayTest(from, to, cb);
  return cb;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::sweep_test_closest
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletClosestHitSweepResult BulletWorld::
sweep_test_closest(BulletShape *shape, const TransformState &from_ts, const TransformState &to_ts, const CollideMask &mask, float penetration) const {

  nassertr(shape, BulletClosestHitSweepResult::empty());
  nassertr(shape->is_convex(), BulletClosestHitSweepResult::empty());
  nassertr(!from_ts.is_invalid(), BulletClosestHitSweepResult::empty());
  nassertr(!to_ts.is_invalid(), BulletClosestHitSweepResult::empty());

  const btConvexShape *convex = (const btConvexShape *) shape->ptr();
  const btVector3 from_pos = LVecBase3f_to_btVector3(from_ts.get_pos());
  const btVector3 to_pos = LVecBase3f_to_btVector3(to_ts.get_pos());
  const btTransform from_trans = LMatrix4f_to_btTrans(from_ts.get_mat());
  const btTransform to_trans = LMatrix4f_to_btTrans(to_ts.get_mat());

  BulletClosestHitSweepResult cb(from_pos, to_pos, mask);
  _world->convexSweepTest(convex, from_trans, to_trans, cb, penetration);
  return cb;  
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::contact_test
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
BulletContactResult BulletWorld::
contact_test(PandaNode *node) const {

  btCollisionObject *obj = get_collision_object(node);

  BulletContactResult cb;

  if (obj) {
    _world->contactTest(obj, cb);
  }

  return cb;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::contact_pair_test
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
BulletContactResult BulletWorld::
contact_test_pair(PandaNode *node0, PandaNode *node1) const {

  btCollisionObject *obj0 = get_collision_object(node0);
  btCollisionObject *obj1 = get_collision_object(node1);

  BulletContactResult cb;

  if (obj0 && obj1) {
    _world->contactPairTest(obj0, obj1, cb);
  }

  return cb;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::get_manifold
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
BulletPersistentManifold *BulletWorld::
get_manifold(int idx) const {

  nassertr(idx < get_num_manifolds(), NULL);

  btPersistentManifold *ptr = _dispatcher->getManifoldByIndexInternal(idx);
  return (ptr) ? new BulletPersistentManifold(ptr) : NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::get_collision_object
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
btCollisionObject *BulletWorld::
get_collision_object(PandaNode *node) {

  if (node->is_of_type(BulletRigidBodyNode::get_class_type())) {
    return ((BulletRigidBodyNode *)node)->get_object();
  }
  else if (node->is_of_type(BulletGhostNode::get_class_type())) {
    return ((BulletGhostNode *)node)->get_object();
  }
  else if (node->is_of_type(BulletCharacterControllerNode::get_class_type())) {
    return ((BulletCharacterControllerNode *)node)->get_ghost();
  }
  else if (node->is_of_type(BulletSoftBodyNode::get_class_type())) {
    return ((BulletSoftBodyNode *)node)->get_object();
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::FilterCallback::needBroadphaseCollision
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
bool BulletWorld::btFilterCallback::
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

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::BroadphaseAlgorithm ostream operator
//  Description:
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::BroadphaseAlgorithm istream operator
//  Description:
////////////////////////////////////////////////////////////////////
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

