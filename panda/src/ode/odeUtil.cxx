// Filename: odeUtil.cxx
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

#include "odeUtil.h"

void OdeUtil::
get_connecting_joint(const OdeBody &body1, const OdeBody &body2, OdeJoint &joint) {
  joint._id = dConnectingJoint(body1.get_id(),body2.get_id());
}

/*
OdeJointList OdeUtil::
get_connecting_joint_list(const OdeBody &body1, const OdeBody &body2) {
  const int max_possible_joints = min(body1.get_num_joints(), body1.get_num_joints());

  dJointID *joint_list_store = new dJointID[max_possible_joints];
  OdeJointList joint_list;

  int num_joints = dConnectingJointList(body1.get_id(),
					body2.get_id(),
					joint_list_store);

  for (int i = 0; i < num_joints; i++) {
    joint_list.push_back(body1.get_world()->get_joint(joint_list_store[i]));
  }
  
  delete joint_list_store;
  return joint_list;
}
*/

int OdeUtil::
are_connected(const OdeBody &body1, const OdeBody &body2) {
  return dAreConnected(body1.get_id(),body2.get_id());
}

int OdeUtil::
are_connected_excluding(const OdeBody &body1,
			const OdeBody &body2,
			const int joint_type) {
  return dAreConnectedExcluding(body1.get_id(),
				body2.get_id(),
				joint_type);
}
  
