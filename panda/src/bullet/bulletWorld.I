/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletWorld.I
 * @author enn0x
 * @date 2010-01-23
 */

/**
 *
 */
INLINE BulletWorld::
~BulletWorld() {

  while (get_num_characters() > 0) {
    remove_character(get_character(0));
  }

  while (get_num_vehicles() > 0) {
    remove_vehicle(get_vehicle(0));
  }

  while (get_num_constraints() > 0) {
    remove_constraint(get_constraint(0));
  }

  while (get_num_rigid_bodies() > 0) {
    remove_rigid_body(get_rigid_body(0));
  }

  while (get_num_soft_bodies() > 0) {
    remove_soft_body(get_soft_body(0));
  }

  while (get_num_ghosts() > 0) {
    remove_ghost(get_ghost(0));
  }

  _info.m_sparsesdf.Reset();

  delete _world;
  delete _solver;
  delete _configuration;
  delete _dispatcher;
  delete _broadphase;
}

/**
 *
 */
INLINE void BulletWorld::
set_debug_node(BulletDebugNode *node) {

  nassertv(node);

  _debug = node;
  _world->setDebugDrawer(&(_debug->_drawer));
}

/**
 *
 */
INLINE void BulletWorld::
clear_debug_node() {

  _debug = NULL;
  _world->setDebugDrawer(NULL);
}

/**
 *
 */
INLINE BulletDebugNode *BulletWorld::
get_debug_node() const {

  return _debug;
}

/**
 *
 */
INLINE bool BulletWorld::
has_debug_node() const {

  return _debug != NULL;
}

/**
 *
 */
INLINE btDynamicsWorld *BulletWorld::
get_world() const {

  return _world;
}

/**
 *
 */
INLINE btBroadphaseInterface *BulletWorld::
get_broadphase() const {

  return _broadphase;
}

/**
 *
 */
INLINE btDispatcher *BulletWorld::
get_dispatcher() const {

  return _dispatcher;
}

/**
 *
 */
INLINE int BulletWorld::
get_num_rigid_bodies() const {

  return _bodies.size();
}

/**
 *
 */
INLINE BulletRigidBodyNode *BulletWorld::
get_rigid_body(int idx) const {

  nassertr(idx >= 0 && idx < (int)_bodies.size(), NULL);
  return _bodies[idx];
}

/**
 *
 */
INLINE int BulletWorld::
get_num_soft_bodies() const {

  return _softbodies.size();
}

/**
 *
 */
INLINE BulletSoftBodyNode *BulletWorld::
get_soft_body(int idx) const {

  nassertr(idx >= 0 && idx < (int)_softbodies.size(), NULL);
  return _softbodies[idx];
}

/**
 *
 */
INLINE int BulletWorld::
get_num_ghosts() const {

  return _ghosts.size();
}

/**
 *
 */
INLINE BulletGhostNode *BulletWorld::
get_ghost(int idx) const {

  nassertr(idx >= 0 && idx < (int)_ghosts.size(), NULL);
  return _ghosts[idx];
}

/**
 *
 */
INLINE int BulletWorld::
get_num_characters() const {

  return _characters.size();
}

/**
 *
 */
INLINE BulletBaseCharacterControllerNode *BulletWorld::
get_character(int idx) const {

  nassertr(idx >= 0 && idx < (int)_characters.size(), NULL);
  return _characters[idx];
}

/**
 *
 */
INLINE int BulletWorld::
get_num_vehicles() const {

  return _vehicles.size();
}

/**
 *
 */
INLINE BulletVehicle *BulletWorld::
get_vehicle(int idx) const {

  nassertr(idx >= 0 && idx < (int)_vehicles.size(), NULL);
  return _vehicles[idx];
}

/**
 *
 */
INLINE int BulletWorld::
get_num_constraints() const {

  return _constraints.size();
}

/**
 *
 */
INLINE BulletConstraint *BulletWorld::
get_constraint(int idx) const {

  nassertr(idx >= 0 && idx < (int)_constraints.size(), NULL);
  return _constraints[idx];
}

/**
 *
 */
INLINE int BulletWorld::
get_num_manifolds() const {

  return _world->getDispatcher()->getNumManifolds();
}
