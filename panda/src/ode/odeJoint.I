// Filename: odeJoint.I
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

INLINE dJointID OdeJoint::
get_id() const {
  return _id;
}

/*
INLINE void OdeJoint::
set_data(void *data) {
  dJointSetData(_id, data);
}

INLINE void *OdeJoint::
get_data() {
  return dJointGetData(_id);
}
*/

INLINE int OdeJoint::
get_joint_type() const {
  return dJointGetType(_id);
}

/*
INLINE void OdeJoint::
set_feedback(dJointFeedback *feedback) {
  dJointSetFeedback(_id, feedback);
}

INLINE dJointFeedback *OdeJoint::
get_feedback() {
  return dJointGetFeedback(_id);
}
*/

INLINE int OdeJoint::
compare_to(const OdeJoint &other) const {
  if (_id != other._id) {
    return _id < other._id ? -1 : 1;
  }
  return 0;
}

INLINE bool OdeJoint::
operator == (const OdeJoint &other) const {
  return _id == other._id;
}
