// Filename: odeJointCollection.cxx
// Created by:  joswilso (27Dec06)
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

#include "odeJointCollection.h"

OdeJointCollection::
OdeJointCollection() {
}

OdeJointCollection::
OdeJointCollection(const OdeJointCollection &copy) :
  _joints(copy._joints) {
}

void OdeJointCollection::
operator = (const OdeJointCollection &copy) {
  _joints = copy._joints;
}

void OdeJointCollection::
add_joint(const OdeJoint &joint) {
  // If the pointer to our internal array is shared by any other
  // OdeJointCollections, we have to copy the array now so we won't
  // inadvertently modify any of our brethren OdeJointCollection
  // objects.

  if (_joints.get_ref_count() > 1) {
    Joints old_joints = _joints;
    _joints = Joints::empty_array(0);
    _joints.v() = old_joints.v();
  }

  _joints.push_back(joint);
}

bool OdeJointCollection::
remove_joint(const OdeJoint &joint) {
  int joint_index = -1;
  for (int i = 0; joint_index == -1 && i < (int)_joints.size(); i++) {
    if (_joints[i] == joint) {
      joint_index = i;
    }
  }

  if (joint_index == -1) {
    // The indicated joint was not a member of the collection.
    return false;
  }

  // If the pointer to our internal array is shared by any other
  // OdeJointCollections, we have to copy the array now so we won't
  // inadvertently modify any of our brethren JointCollection
  // objects.

  if (_joints.get_ref_count() > 1) {
    Joints old_joints = _joints;
    _joints = Joints::empty_array(0);
    _joints.v() = old_joints.v();
  }

  _joints.erase(_joints.begin() + joint_index);
  return true;
}

void OdeJointCollection::
add_joints_from(const OdeJointCollection &other) {
  int other_num_joints = other.get_num_joints();
  for (int i = 0; i < other_num_joints; i++) {
    add_joint(other.get_joint(i));
  }
}

void OdeJointCollection::
remove_joints_from(const OdeJointCollection &other) {
  Joints new_joints;
  int num_joints = get_num_joints();
  for (int i = 0; i < num_joints; i++) {
    OdeJoint joint = get_joint(i);
    if (!other.has_joint(joint)) {
      new_joints.push_back(joint);
    }
  }
  _joints = new_joints;
}

void OdeJointCollection::
remove_duplicate_joints() {
  Joints new_joints;

  int num_joints = get_num_joints();
  for (int i = 0; i < num_joints; i++) {
    OdeJoint joint = get_joint(i);
    bool duplicated = false;

    for (int j = 0; j < i && !duplicated; j++) {
      duplicated = (joint == get_joint(j));
    }

    if (!duplicated) {
      new_joints.push_back(joint);
    }
  }

  _joints = new_joints;
}

bool OdeJointCollection::
has_joint(const OdeJoint &joint) const {
  for (int i = 0; i < get_num_joints(); i++) {
    if (joint == get_joint(i)) {
      return true;
    }
  }
  return false;
}

void OdeJointCollection::
clear() {
  _joints.clear();
}

bool OdeJointCollection::
is_empty() const {
  return _joints.empty();
}

int OdeJointCollection::
get_num_joints() const {
  return _joints.size();
}

OdeJoint OdeJointCollection::
get_joint(int index) const {
  nassertr(index >= 0 && index < (int)_joints.size(), OdeJoint());
  return _joints[index];
}

OdeJoint OdeJointCollection::
operator [] (int index) const {
  return get_joint(index);
}

////////////////////////////////////////////////////////////////////
//     Function: OdeJointCollection::size
//       Access: Published
//  Description: Returns the number of joints in the collection.  This
//               is the same thing as get_num_joints().
////////////////////////////////////////////////////////////////////
int OdeJointCollection::
size() const {
  return _joints.size();
}
