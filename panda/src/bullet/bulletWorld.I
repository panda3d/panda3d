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
INLINE BulletDebugNode *BulletWorld::
get_debug_node() const {

  return _debug;
}

/**
 *
 */
INLINE bool BulletWorld::
has_debug_node() const {

  return _debug != nullptr;
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

