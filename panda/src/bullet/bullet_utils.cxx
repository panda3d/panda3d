// Filename: bullet_utils.h
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

#include "bullet_utils.h"

#include "transformState.h"

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3_to_btVector3
//  Description: 
////////////////////////////////////////////////////////////////////
btVector3 LVecBase3_to_btVector3(const LVecBase3 &v) {

  return btVector3((btScalar)v.get_x(),
                   (btScalar)v.get_y(),
                   (btScalar)v.get_z());
}

////////////////////////////////////////////////////////////////////
//     Function: btVector3_to_LVecBase3
//  Description: 
////////////////////////////////////////////////////////////////////
LVecBase3 btVector3_to_LVecBase3(const btVector3 &v) {

  return LVecBase3((PN_stdfloat)v.getX(),
                   (PN_stdfloat)v.getY(),
                   (PN_stdfloat)v.getZ());
}

////////////////////////////////////////////////////////////////////
//     Function: btVector3_to_LVector3
//  Description: 
////////////////////////////////////////////////////////////////////
LVector3 btVector3_to_LVector3(const btVector3 &v) {

  return LVector3((PN_stdfloat)v.getX(),
                  (PN_stdfloat)v.getY(),
                  (PN_stdfloat)v.getZ());
}

////////////////////////////////////////////////////////////////////
//     Function: btVector3_to_LPoint3
//  Description: 
////////////////////////////////////////////////////////////////////
LPoint3 btVector3_to_LPoint3(const btVector3 &p) {

  return LPoint3((PN_stdfloat)p.getX(),
                 (PN_stdfloat)p.getY(),
                 (PN_stdfloat)p.getZ());
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3_to_btMatrix3x3
//  Description: 
////////////////////////////////////////////////////////////////////
btMatrix3x3 LMatrix3_to_btMatrix3x3(const LMatrix3 &m) {

  btMatrix3x3 result;
  result.setFromOpenGLSubMatrix((const btScalar *)m.get_data());
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: btMatrix3x3_to_LMatrix3
//  Description: 
////////////////////////////////////////////////////////////////////
LMatrix3 btMatrix3x3_to_LMatrix3(const btMatrix3x3 &m) {

  btScalar cells[9];
  m.getOpenGLSubMatrix(cells);
  return LMatrix3((PN_stdfloat)cells[0], (PN_stdfloat)cells[1], (PN_stdfloat)cells[2],
                  (PN_stdfloat)cells[3], (PN_stdfloat)cells[4], (PN_stdfloat)cells[5],
                  (PN_stdfloat)cells[6], (PN_stdfloat)cells[7], (PN_stdfloat)cells[8]);
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion_to_btQuat
//  Description: 
////////////////////////////////////////////////////////////////////
btQuaternion LQuaternion_to_btQuat(const LQuaternion &q) {

  return btQuaternion((btScalar)q.get_i(),
                      (btScalar)q.get_j(),
                      (btScalar)q.get_k(),
                      (btScalar)q.get_r());
}

////////////////////////////////////////////////////////////////////
//     Function: btQuat_to_LQuaternion
//  Description: 
////////////////////////////////////////////////////////////////////
LQuaternion btQuat_to_LQuaternion(const btQuaternion &q) {

  return LQuaternion((PN_stdfloat)q.getW(),
                     (PN_stdfloat)q.getX(),
                     (PN_stdfloat)q.getY(),
                     (PN_stdfloat)q.getZ());
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4_to_btTrans
//  Description: 
////////////////////////////////////////////////////////////////////
btTransform LMatrix4_to_btTrans(const LMatrix4 &m) {

  LQuaternion quat;
  quat.set_from_matrix(m.get_upper_3());

  btQuaternion btq = LQuaternion_to_btQuat(quat);
  btVector3 btv = LVecBase3_to_btVector3(m.get_row3(3));

  return btTransform(btq, btv);
}

////////////////////////////////////////////////////////////////////
//     Function: btTrans_to_LMatrix4
//  Description: 
////////////////////////////////////////////////////////////////////
LMatrix4 btTrans_to_LMatrix4(const btTransform &trans) {

  return TransformState::make_pos_quat_scale(
    btVector3_to_LVector3(trans.getOrigin()),
    btQuat_to_LQuaternion(trans.getRotation()),
    LVector3(1.0f, 1.0f, 1.0f))->get_mat();
}

////////////////////////////////////////////////////////////////////
//     Function: btTrans_to_TransformState
//  Description: 
////////////////////////////////////////////////////////////////////
CPT(TransformState) btTrans_to_TransformState(const btTransform &trans, const LVecBase3 &scale) {

  LVecBase3 pos = btVector3_to_LVector3(trans.getOrigin());
  LQuaternion quat = btQuat_to_LQuaternion(trans.getRotation());

  return TransformState::make_pos_quat_scale(pos, quat, scale);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState_to_btTrans
//  Description: 
////////////////////////////////////////////////////////////////////
btTransform TransformState_to_btTrans(CPT(TransformState) ts) {

  ts = ts->set_scale(1.0);

  LMatrix4 m = ts->get_mat();

  LQuaternion quat;
  quat.set_from_matrix(m.get_upper_3());

  btQuaternion btq = LQuaternion_to_btQuat(quat);
  btVector3 btv = LVecBase3_to_btVector3(m.get_row3(3));

  return btTransform(btq, btv);
}

////////////////////////////////////////////////////////////////////
//     Function: get_default_up_axis
//  Description: 
////////////////////////////////////////////////////////////////////
BulletUpAxis get_default_up_axis() {

  switch (get_default_coordinate_system()) {

  case CS_yup_right:
  case CS_yup_left:
    return Y_up;

  case CS_zup_right:
  case CS_zup_left:
    return Z_up;

  default:
    return Z_up;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: get_node_transform
//  Description: 
////////////////////////////////////////////////////////////////////
void get_node_transform(btTransform &trans, PandaNode *node) {

  // Get TS
  CPT(TransformState) ts;
  if (node->get_num_parents() == 0) {
    ts = node->get_transform();
  }
  else {
    NodePath np = NodePath::any_path(node);
    ts = np.get_net_transform();
  }

  // Remove scale from TS, since scale fudges the orientation
  ts = ts->set_scale(1.0);

  // Convert
  LMatrix4 m = ts->get_mat();

  LQuaternion quat;
  quat.set_from_matrix(m.get_upper_3());

  btQuaternion btq = LQuaternion_to_btQuat(quat);
  btVector3 btv = LVecBase3_to_btVector3(m.get_row3(3));

  trans.setRotation(btq);
  trans.setOrigin(btv);
}

////////////////////////////////////////////////////////////////////
//     Function: get_bullet_version
//  Description: Returns the version of the linked Bullet library.
////////////////////////////////////////////////////////////////////
int get_bullet_version() {

  return BT_BULLET_VERSION;
}

