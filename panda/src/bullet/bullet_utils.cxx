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
//     Function: LVecBase3f_to_btVector3
//  Description: 
////////////////////////////////////////////////////////////////////
btVector3 LVecBase3f_to_btVector3(const LVecBase3f &v) {

  return btVector3(v.get_x(), v.get_y(), v.get_z());
}

////////////////////////////////////////////////////////////////////
//     Function: btVector3_to_LVecBase3f
//  Description: 
////////////////////////////////////////////////////////////////////
LVecBase3f btVector3_to_LVecBase3f(const btVector3 &v) {

  return LVecBase3f(v.getX(), v.getY(), v.getZ());
}

////////////////////////////////////////////////////////////////////
//     Function: btVector3_to_LVector3f
//  Description: 
////////////////////////////////////////////////////////////////////
LVector3f btVector3_to_LVector3f(const btVector3 &v) {

  return LVector3f(v.getX(), v.getY(), v.getZ());
}

////////////////////////////////////////////////////////////////////
//     Function: btVector3_to_LPoint3f
//  Description: 
////////////////////////////////////////////////////////////////////
LPoint3f btVector3_to_LPoint3f(const btVector3 &p) {

  return LPoint3f(p.getX(), p.getY(), p.getZ());
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3f_to_btMatrix3x3
//  Description: 
////////////////////////////////////////////////////////////////////
btMatrix3x3 LMatrix3f_to_btMatrix3x3(const LMatrix3f &m) {

  btMatrix3x3 result;
  result.setFromOpenGLSubMatrix(m.get_data());
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: btMatrix3x3_to_LMatrix3f
//  Description: 
////////////////////////////////////////////////////////////////////
LMatrix3f btMatrix3x3_to_LMatrix3f(const btMatrix3x3 &m) {

  float cells[9];
  m.getOpenGLSubMatrix(cells);
  return LMatrix3f(cells[0], cells[1], cells[2],
                   cells[3], cells[4], cells[5],
                   cells[6], cells[7], cells[8]);
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternionf_to_btQuat
//  Description: 
////////////////////////////////////////////////////////////////////
btQuaternion LQuaternionf_to_btQuat(const LQuaternionf &q) {

  return btQuaternion(q.get_i(), q.get_j(), q.get_k(), q.get_r());
}

////////////////////////////////////////////////////////////////////
//     Function: btQuat_to_LQuaternionf
//  Description: 
////////////////////////////////////////////////////////////////////
LQuaternionf btQuat_to_LQuaternionf(const btQuaternion &q) {

  return LQuaternionf(q.getW(), q.getX(), q.getY(), q.getZ());
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4f_to_btTrans
//  Description: 
////////////////////////////////////////////////////////////////////
btTransform LMatrix4f_to_btTrans(const LMatrix4f &m) {

  LQuaternionf quat;
  quat.set_from_matrix(m.get_upper_3());

  btQuaternion btq = LQuaternionf_to_btQuat(quat);
  btVector3 btv = LVecBase3f_to_btVector3(m.get_row3(3));

  return btTransform(btq, btv);
}

////////////////////////////////////////////////////////////////////
//     Function: btTrans_to_LMatrix4f
//  Description: 
////////////////////////////////////////////////////////////////////
LMatrix4f btTrans_to_LMatrix4f(const btTransform &trans) {

  return TransformState::make_pos_quat_scale(
    btVector3_to_LVector3f(trans.getOrigin()),
    btQuat_to_LQuaternionf(trans.getRotation()),
    LVector3f(1.0f, 1.0f, 1.0f))->get_mat();
}

////////////////////////////////////////////////////////////////////
//     Function: btTrans_to_TransformState
//  Description: 
////////////////////////////////////////////////////////////////////
CPT(TransformState) btTrans_to_TransformState(const btTransform &trans, const LVecBase3f &scale) {

  LVecBase3f pos = btVector3_to_LVector3f(trans.getOrigin());
  LQuaternionf quat = btQuat_to_LQuaternionf(trans.getRotation());

  return TransformState::make_pos_quat_scale(pos, quat, scale);
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

  LMatrix4f m;

  if (node->get_num_parents() == 0) {
    m = node->get_transform()->get_mat();
  }
  else {
    NodePath np = NodePath::any_path(node);
    m = np.get_net_transform()->get_mat();
  }

  LQuaternionf quat;
  quat.set_from_matrix(m.get_upper_3());

  btQuaternion btq = LQuaternionf_to_btQuat(quat);
  btVector3 btv = LVecBase3f_to_btVector3(m.get_row3(3));

  trans.setRotation(btq);
  trans.setOrigin(btv);
}

