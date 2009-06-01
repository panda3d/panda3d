// Filename: physxJointDesc.I
// Created by:  pratt (Jun 20, 2006)
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
//     Function : is_valid
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE bool PhysxJointDesc::
is_valid() const {
  nassertr(nJointDesc != NULL, false);

  return nJointDesc->isValid();
}

////////////////////////////////////////////////////////////////////
//     Function : set_to_default
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxJointDesc::
set_to_default() {
  nassertv(nJointDesc != NULL);

  nJointDesc->setToDefault();
}


////////////////////////////////////////////////////////////////////
//     Function : get_joint_flags
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE unsigned int PhysxJointDesc::
get_joint_flags() const {
  nassertr(nJointDesc != NULL, -1);

  return nJointDesc->jointFlags;
}

////////////////////////////////////////////////////////////////////
//     Function : get_max_force
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE float PhysxJointDesc::
get_max_force() const {
  nassertr(nJointDesc != NULL, -1.0f);

  return nJointDesc->maxForce;
}

////////////////////////////////////////////////////////////////////
//     Function : get_max_torque
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE float PhysxJointDesc::
get_max_torque() const {
  nassertr(nJointDesc != NULL, -1.0f);

  return nJointDesc->maxTorque;
}

////////////////////////////////////////////////////////////////////
//     Function : get_name
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE const char * PhysxJointDesc::
get_name() const {
  nassertr(nJointDesc != NULL, NULL);

  return nJointDesc->name;
}

////////////////////////////////////////////////////////////////////
//     Function : set_joint_flags
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxJointDesc::
set_joint_flags(unsigned int value) {
  nassertv(nJointDesc != NULL);

  nJointDesc->jointFlags = value;
}

////////////////////////////////////////////////////////////////////
//     Function : set_max_force
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxJointDesc::
set_max_force(float value) {
  nassertv(nJointDesc != NULL);

  nJointDesc->maxForce = value;
}

////////////////////////////////////////////////////////////////////
//     Function : set_max_torque
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxJointDesc::
set_max_torque(float value) {
  nassertv(nJointDesc != NULL);

  nJointDesc->maxTorque = value;
}

////////////////////////////////////////////////////////////////////
//     Function : set_name
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxJointDesc::
set_name(const char * value) {
  nassertv(nJointDesc != NULL);

  // Because the PhysX engine does not store its own copy of names,
  // we keep a local copy on this instance.  Otherwise, it would be
  // very easy for names to be declared in python and then
  // invalidated when the string is reclaimed from reference
  // counting.
  _name_store = value;
  nJointDesc->name = _name_store.c_str();
}


