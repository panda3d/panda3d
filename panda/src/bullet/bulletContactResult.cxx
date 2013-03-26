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

BulletContact BulletContactResult::_empty;

////////////////////////////////////////////////////////////////////
//     Function: BulletContactResult::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
BulletContactResult::
BulletContactResult() : btCollisionWorld::ContactResultCallback() {

}

#if BT_BULLET_VERSION >= 281
////////////////////////////////////////////////////////////////////
//     Function: BulletContactResult::addSingleResult
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
btScalar BulletContactResult::
addSingleResult(btManifoldPoint &mp,
                const btCollisionObjectWrapper *wrap0, int part_id0, int idx0,
                const btCollisionObjectWrapper *wrap1, int part_id1, int idx1) {

  BulletContact contact;

  contact._mp = mp;
  contact._obj0 = wrap0->getCollisionObject();
  contact._obj1 = wrap1->getCollisionObject();
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

  contact._mp = mp;
  contact._obj0 = obj0;
  contact._obj1 = obj1;
  contact._part_id0 = part_id0;
  contact._part_id1 = part_id1;
  contact._idx0 = idx0;
  contact._idx1 = idx1;

  _contacts.push_back(contact);

  return 1.0f;
}
#endif

