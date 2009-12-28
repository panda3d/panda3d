// Filename: physxManager.I
// Created by:  enn0x (01Sep09)
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
//    Function: PhysxManager::get_sdk
//      Access: Public
// Description: Returns a pointer to the NxPhysicsSDK.
////////////////////////////////////////////////////////////////////
INLINE NxPhysicsSDK *PhysxManager::
get_sdk() const {

  return _sdk;
}

////////////////////////////////////////////////////////////////////
//    Function: PhysxManager::vec3_to_nxVec3
//      Access: Public
// Description: Converts from LVector3f to NxVec3.
////////////////////////////////////////////////////////////////////
INLINE NxVec3 PhysxManager::
vec3_to_nxVec3(const LVector3f &v) {

  return NxVec3(v.get_x(), v.get_y(), v.get_z());
}

////////////////////////////////////////////////////////////////////
//    Function: PhysxManager::nxVec3_to_vec3
//      Access: Public
// Description: Converts from NxVec3 to LVector3f.
////////////////////////////////////////////////////////////////////
INLINE LVector3f PhysxManager::
nxVec3_to_vec3(const NxVec3 &v) {

  return LVector3f(v.x, v.y, v.z);
}

////////////////////////////////////////////////////////////////////
//    Function: PhysxManager::vec3_to_nxExtVec3
//      Access: Public
// Description: Converts from LVector3f to NxExtendedVec3.
////////////////////////////////////////////////////////////////////
INLINE NxExtendedVec3 PhysxManager::
vec3_to_nxExtVec3(const LVector3f &v) {

  return NxExtendedVec3(v.get_x(), v.get_y(), v.get_z());
}

////////////////////////////////////////////////////////////////////
//    Function: PhysxManager::nxExtVec3_to_vec3
//      Access: Public
// Description: Converts from NxExtendedVec3 to LVector3f.
////////////////////////////////////////////////////////////////////
INLINE LVector3f PhysxManager::
nxExtVec3_to_vec3(const NxExtendedVec3 &v) {

  return LVector3f(v.x, v.y, v.z);
}

////////////////////////////////////////////////////////////////////
//    Function: PhysxManager::point3_to_nxVec3
//      Access: Public
// Description: Converts from LPoint3f to NxVec3.
////////////////////////////////////////////////////////////////////
INLINE NxVec3 PhysxManager::
point3_to_nxVec3(const LPoint3f &p) {

  return NxVec3(p.get_x(), p.get_y(), p.get_z());
}

////////////////////////////////////////////////////////////////////
//    Function: PhysxManager::nxVec3_to_point3
//      Access: Public
// Description: Converts from NxVec3 to LPoint3f.
////////////////////////////////////////////////////////////////////
INLINE LPoint3f PhysxManager::
nxVec3_to_point3(const NxVec3 &p) {

  return LPoint3f(p.x, p.y, p.z);
}

////////////////////////////////////////////////////////////////////
//    Function: PhysxManager::point3_to_nxExtVec3
//      Access: Public
// Description: Converts from LPoint3f to NxExtendedVec3.
////////////////////////////////////////////////////////////////////
INLINE NxExtendedVec3 PhysxManager::
point3_to_nxExtVec3(const LPoint3f &p) {

  return NxExtendedVec3(p.get_x(), p.get_y(), p.get_z());
}

////////////////////////////////////////////////////////////////////
//    Function: PhysxManager::nxExtVec3_to_point3
//      Access: Public
// Description: Converts from NxExtendedVec3 to LPoint3f.
////////////////////////////////////////////////////////////////////
INLINE LPoint3f PhysxManager::
nxExtVec3_to_point3(const NxExtendedVec3 &p) {

  return LPoint3f(p.x, p.y, p.z);
}

////////////////////////////////////////////////////////////////////
//    Function: PhysxManager::quat_to_nxQuat
//      Access: Public
// Description: Converts from LQuaternionf to NxQuat.
////////////////////////////////////////////////////////////////////
INLINE NxQuat PhysxManager::
quat_to_nxQuat(const LQuaternionf &q) {

  NxQuat nxq;
  nxq.setXYZW(q.get_i(), q.get_j(), q.get_k(), q.get_r());
  return nxq;
}

////////////////////////////////////////////////////////////////////
//    Function: PhysxManager::nxQuat_to_quat
//      Access: Public
// Description: Converts from NxQuat to LQuaternionf.
////////////////////////////////////////////////////////////////////
INLINE LQuaternionf PhysxManager::
nxQuat_to_quat(const NxQuat &q) {

  return LQuaternionf(q.w, q.x, q.y, q.z);
}

////////////////////////////////////////////////////////////////////
//    Function: PhysxManager::mat4_to_nxMat34
//      Access: Public
// Description: Converts from LMatrix4f to NxMat34.
////////////////////////////////////////////////////////////////////
INLINE NxMat34 PhysxManager::
mat4_to_nxMat34(const LMatrix4f &m) {

  NxMat33 mat = mat3_to_nxMat33(m.get_upper_3());
  NxVec3 v = vec3_to_nxVec3(m.get_row3(3));
  return NxMat34(mat, v);
}

////////////////////////////////////////////////////////////////////
//    Function: PhysxManager::nxMat34_to_mat4
//      Access: Public
// Description: Converts from NxMat34 to LMatrix4f.
////////////////////////////////////////////////////////////////////
INLINE LMatrix4f PhysxManager::
nxMat34_to_mat4(const NxMat34 &m) {

  return LMatrix4f(nxMat33_to_mat3(m.M), nxVec3_to_vec3(m.t));
}

////////////////////////////////////////////////////////////////////
//    Function: PhysxManager::mat3_to_nxMat33
//      Access: Public
// Description: Converts from LMatrix3f to NxMat33.
////////////////////////////////////////////////////////////////////
INLINE NxMat33 PhysxManager::
mat3_to_nxMat33(const LMatrix3f &m) {

  NxMat33 mat;
  mat.setColumnMajor(m.get_data());
  return mat;
}

////////////////////////////////////////////////////////////////////
//    Function: PhysxManager::nxMat33_to_mat3
//      Access: Public
// Description: Converts from NxMat33 to LMatrix3f.
////////////////////////////////////////////////////////////////////
INLINE LMatrix3f PhysxManager::
nxMat33_to_mat3(const NxMat33 &m) {

  float cells[9];
  m.getColumnMajor(cells);
  return LMatrix3f(cells[0], cells[1], cells[2],
                   cells[3], cells[4], cells[5],
                   cells[6], cells[7], cells[8]);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxManager::update_vec3_from_nxVec3
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxManager::
update_vec3_from_nxVec3(LVector3f &v, const NxVec3 &nVec) {

  v.set_x(nVec.x);
  v.set_y(nVec.y);
  v.set_z(nVec.z);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxManager::update_point3_from_nxVec3
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxManager::
update_point3_from_nxVec3(LPoint3f &p, const NxVec3 &nVec) {

  p.set_x(nVec.x);
  p.set_y(nVec.y);
  p.set_z(nVec.z);
}


////////////////////////////////////////////////////////////////////
//     Function: PhysxManager::ls
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxManager::
ls() const {

  ls(nout);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxManager::ls
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxManager::
ls(ostream &out, int indent_level) const {

  indent(out, indent_level) << "PhysxManager\n";

  _scenes.ls(out, indent_level);
  _heightfields.ls(out, indent_level);
  _convex_meshes.ls(out, indent_level);
  _triangle_meshes.ls(out, indent_level);
}

