// Filename: bulletContactResult.cxx
// Created by:  enn0x (08Mar10)
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

#include "bulletContactResult.h"

btManifoldPoint BulletContact::_empty;
BulletContact BulletContactResult::_empty;

////////////////////////////////////////////////////////////////////
//     Function: BulletContact::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletContact::
BulletContact() : _mp(_empty) {

  _node0 = NULL;
  _node1 = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletContact::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletContact::
BulletContact(const BulletContact &other) : _mp(other._mp) {

  _node0 = other._node0;
  _node1 = other._node1;
  _part_id0 = other._part_id0;
  _part_id1 = other._part_id1;
  _idx0 = other._idx0;
  _idx1 = other._idx1;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletContactResult::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
BulletContactResult::
BulletContactResult() : btCollisionWorld::ContactResultCallback() {

#if BT_BULLET_VERSION >= 281
  _filter_cb = NULL;
  _filter_proxy = NULL;
  _filter_set = false;
#endif
}

#if BT_BULLET_VERSION >= 281
////////////////////////////////////////////////////////////////////
//     Function: BulletContactResult::use_filter
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void BulletContactResult::
use_filter(btOverlapFilterCallback *cb, btBroadphaseProxy *proxy) {

  nassertv(cb);
  nassertv(proxy);

  _filter_cb = cb;
  _filter_proxy = proxy;
  _filter_set = true;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletContactResult::needsCollision
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
bool BulletContactResult::
needsCollision(btBroadphaseProxy *proxy0) const {

  if (_filter_set) {
    return _filter_cb->needBroadphaseCollision(proxy0, _filter_proxy);
  }
  else {
    return true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BulletContactResult::addSingleResult
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
btScalar BulletContactResult::
addSingleResult(btManifoldPoint &mp,
                const btCollisionObjectWrapper *wrap0, int part_id0, int idx0,
                const btCollisionObjectWrapper *wrap1, int part_id1, int idx1) {

  const btCollisionObject *obj0 = wrap0->getCollisionObject();
  const btCollisionObject *obj1 = wrap1->getCollisionObject();

  BulletContact contact;

  contact._mp = BulletManifoldPoint(mp);
  contact._node0 = obj0 ? (PandaNode *)obj0->getUserPointer() : NULL;
  contact._node1 = obj1 ? (PandaNode *)obj1->getUserPointer() : NULL;
  contact._part_id0 = part_id0;
  contact._part_id1 = part_id1;
  contact._idx0 = idx0;
  contact._idx1 = idx1;

  _contacts.push_back(contact);

  return 1.0f;
}
#else
////////////////////////////////////////////////////////////////////
//     Function: BulletContactResult::addSingleResult
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
btScalar BulletContactResult::
addSingleResult(btManifoldPoint &mp,
                const btCollisionObject *obj0, int part_id0, int idx0,
                const btCollisionObject *obj1, int part_id1, int idx1) {

  BulletContact contact;

  contact._mp = BulletManifoldPoint(mp);
  contact._node0 = obj0 ? (PandaNode *)obj0->getUserPointer() : NULL;
  contact._node1 = obj1 ? (PandaNode *)obj1->getUserPointer() : NULL;
  contact._part_id0 = part_id0;
  contact._part_id1 = part_id1;
  contact._idx0 = idx0;
  contact._idx1 = idx1;

  _contacts.push_back(contact);

  return 1.0f;
}
#endif

