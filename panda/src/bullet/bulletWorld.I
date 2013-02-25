// Filename: bulletWorld.I
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


////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::set_debug_node
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletWorld::
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
INLINE void BulletWorld::
clear_debug_node() {

  _debug = NULL;
  _world->setDebugDrawer(NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::get_debug_node
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletDebugNode *BulletWorld::
get_debug_node() const {

  return _debug;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::get_world
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE btDynamicsWorld *BulletWorld::
get_world() const {

  return _world;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::get_broadphase
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE btBroadphaseInterface *BulletWorld::
get_broadphase() const {

  return _broadphase;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::get_dispatcher
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE btDispatcher *BulletWorld::
get_dispatcher() const {

  return _dispatcher;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::get_num_rigid_bodies
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int BulletWorld::
get_num_rigid_bodies() const {

  return _bodies.size();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::get_rigid_body
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletRigidBodyNode *BulletWorld::
get_rigid_body(int idx) const {

  nassertr(idx >= 0 && idx < (int)_bodies.size(), NULL);
  return _bodies[idx];
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::get_num_soft_bodies
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int BulletWorld::
get_num_soft_bodies() const {

  return _softbodies.size();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::get_soft_body
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletSoftBodyNode *BulletWorld::
get_soft_body(int idx) const {

  nassertr(idx >= 0 && idx < (int)_softbodies.size(), NULL);
  return _softbodies[idx];
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::get_num_ghosts
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int BulletWorld::
get_num_ghosts() const {

  return _ghosts.size();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::get_ghost
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletGhostNode *BulletWorld::
get_ghost(int idx) const {

  nassertr(idx >= 0 && idx < (int)_ghosts.size(), NULL);
  return _ghosts[idx];
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::get_num_characters
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int BulletWorld::
get_num_characters() const {

  return _characters.size();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::get_character
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletBaseCharacterControllerNode *BulletWorld::
get_character(int idx) const {

  nassertr(idx >= 0 && idx < (int)_characters.size(), NULL);
  return _characters[idx];
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::get_num_vehicles
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int BulletWorld::
get_num_vehicles() const {

  return _vehicles.size();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::get_vehicle
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletVehicle *BulletWorld::
get_vehicle(int idx) const {

  nassertr(idx >= 0 && idx < (int)_vehicles.size(), NULL);
  return _vehicles[idx];
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::get_num_constraints
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int BulletWorld::
get_num_constraints() const {

  return _constraints.size();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::get_constraint
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletConstraint *BulletWorld::
get_constraint(int idx) const {

  nassertr(idx >= 0 && idx < (int)_constraints.size(), NULL);
  return _constraints[idx];
}

////////////////////////////////////////////////////////////////////
//     Function: BulletWorld::get_num_manifolds
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int BulletWorld::
get_num_manifolds() const {

  return _world->getDispatcher()->getNumManifolds();
}


