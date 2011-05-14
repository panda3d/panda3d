// Filename: bulletGhostNode.cxx
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

#include "bulletGhostNode.h"
#include "bulletShape.h"

TypeHandle BulletGhostNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BulletGhostNode::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletGhostNode::
BulletGhostNode(const char *name) : BulletBodyNode(name) {

  // Setup initial transform
  btTransform trans = btTransform::getIdentity();

  // Setup ghost object
  _ghost = new btPairCachingGhostObject();
  _ghost->setUserPointer(this);
  _ghost->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
  _ghost->setWorldTransform(trans);
  _ghost->setInterpolationWorldTransform(trans);
  _ghost->setCollisionShape(_shape);

  // Autosync is off by default
  _sync_transform = false;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletGhostNode::get_object
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
btCollisionObject *BulletGhostNode::
get_object() const {

  return _ghost;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletGhostNode::parents_changed
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void BulletGhostNode::
parents_changed() {

  // Enable autosync if one of the parents is suited for this
  Parents parents = get_parents();
  for (int i=0; i < parents.get_num_parents(); ++i) {
    PandaNode *parent = parents.get_parent(i);
    TypeHandle type = parent->get_type();

    if (BulletRigidBodyNode::get_class_type() == type ||
        BulletSoftBodyNode::get_class_type() == type ||
        BulletGhostNode::get_class_type() == type ||
        BulletCharacterControllerNode::get_class_type() == type) {
      _sync_transform = true;
      return;
    }
  }

  // None of the parents is suited for autosync
  _sync_transform = false;

  transform_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletGhostNode::transform_changed
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void BulletGhostNode::
transform_changed() {

  if (_disable_transform_changed) return;

  btTransform trans;
  get_node_transform(trans, this);
  _ghost->setWorldTransform(trans);
  _ghost->setInterpolationWorldTransform(trans);

  BulletBodyNode::transform_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletGhostNode::pre_step
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void BulletGhostNode::
pre_step() {

  if (_sync_transform) {
    transform_changed();
  }
}

