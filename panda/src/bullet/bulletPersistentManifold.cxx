// Filename: bulletPersistentManifold.cxx
// Created by:  enn0x (07Mar10)
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

#include "bulletPersistentManifold.h"
#include "bulletManifoldPoint.h"

////////////////////////////////////////////////////////////////////
//     Function: BulletPersistentManifold::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BulletPersistentManifold::
BulletPersistentManifold(btPersistentManifold *manifold) : _manifold(manifold) {

}

////////////////////////////////////////////////////////////////////
//     Function: BulletPersistentManifold::get_contact_breaking_threshold
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletPersistentManifold::
get_contact_breaking_threshold() const {

  return (PN_stdfloat)_manifold->getContactBreakingThreshold();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletPersistentManifold::get_contact_processing_threshold
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletPersistentManifold::
get_contact_processing_threshold() const {

  return (PN_stdfloat)_manifold->getContactProcessingThreshold();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletPersistentManifold::set_suspension_stiffness
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletPersistentManifold::
clear_manifold() {

  _manifold->clearManifold();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletPersistentManifold::get_node0
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
const PandaNode *BulletPersistentManifold::
get_node0() {

#if BT_BULLET_VERSION >= 281
  const btCollisionObject *obj = _manifold->getBody0();
#else
  const btCollisionObject *obj = (btCollisionObject *)_manifold->getBody0();
#endif

  return (obj) ? (const PandaNode *)obj->getUserPointer(): NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletPersistentManifold::get_node1
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
const PandaNode *BulletPersistentManifold::
get_node1() {

#if BT_BULLET_VERSION >= 281
  const btCollisionObject *obj = _manifold->getBody1();
#else
  const btCollisionObject *obj = (btCollisionObject *)_manifold->getBody1();
#endif

  return (obj) ? (const PandaNode *)obj->getUserPointer(): NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletPersistentManifold::get_num_manifold_points
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
int BulletPersistentManifold::
get_num_manifold_points() const {

  return _manifold->getNumContacts();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletPersistentManifold::get_manifold_point
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletManifoldPoint *BulletPersistentManifold::
get_manifold_point(int idx) const {

  nassertr(idx < _manifold->getNumContacts(), NULL)

  return new BulletManifoldPoint(_manifold->getContactPoint(idx));
}

