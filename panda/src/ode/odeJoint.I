// Filename: odeJoint.I
// Created by:  joswilso (27Dec06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
