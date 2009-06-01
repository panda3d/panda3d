// Filename: physxManager.I
// Created by:  pratt (Apr 6, 2006)
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
//    Function : nxVec3_to_lVecBase3
//      Access : Public
// Description : convert NxVec3 to LVecBase3f
////////////////////////////////////////////////////////////////////
INLINE LVecBase3f PhysxManager::
nxVec3_to_lVecBase3(const NxVec3 &vec) {
  return LVecBase3f(vec.x, vec.y, vec.z);
}

////////////////////////////////////////////////////////////////////
//    Function : nxMat33_to_lMatrix3
//      Access : Public
// Description : convert NxMat33 to LMatrix3f
////////////////////////////////////////////////////////////////////
INLINE LMatrix3f PhysxManager::
nxMat33_to_lMatrix3(const NxMat33 &mat) {
  float cells[9];
  mat.getColumnMajor(cells);
  return LMatrix3f(
    cells[0], cells[1], cells[2],
    cells[3], cells[4], cells[5],
    cells[6], cells[7], cells[8]
  );
}

////////////////////////////////////////////////////////////////////
//    Function : nxMat34_to_lMatrix4
//      Access : Public
// Description : convert NxMat34 to LMatrix4f
////////////////////////////////////////////////////////////////////
INLINE LMatrix4f PhysxManager::
nxMat34_to_lMatrix4(const NxMat34 &mat) {
  return LMatrix4f(nxMat33_to_lMatrix3(mat.M), nxVec3_to_lVecBase3(mat.t));
}

////////////////////////////////////////////////////////////////////
//    Function : nxQuat_to_lQuaternion
//      Access : Public
// Description : convert NxQuat to LQuaternionf
////////////////////////////////////////////////////////////////////
INLINE LQuaternionf PhysxManager::
nxQuat_to_lQuaternion(const NxQuat &quat) {
  return LQuaternionf(quat.w, quat.x, quat.y, quat.z);
}

////////////////////////////////////////////////////////////////////
//    Function : lVecBase3_to_nxVec3
//      Access : Public
// Description : convert LVecBase3f to NxVec3
////////////////////////////////////////////////////////////////////
INLINE NxVec3 PhysxManager::
lVecBase3_to_nxVec3(const LVecBase3f &vec) {
  return NxVec3(vec.get_x(), vec.get_y(), vec.get_z());
}

////////////////////////////////////////////////////////////////////
//    Function : lMatrix3_to_nxMat33
//      Access : Public
// Description : convert LMatrix3f to NxMat33
////////////////////////////////////////////////////////////////////
INLINE NxMat33 PhysxManager::
lMatrix3_to_nxMat33(const LMatrix3f &mat) {
  const float *cells = mat.get_data();
  NxMat33 nMat;
  nMat.setColumnMajor(cells);
  return nMat;
}

////////////////////////////////////////////////////////////////////
//    Function : lMatrix4_to_nxMat34
//      Access : Public
// Description : convert LMatrix4f to NxMat34
////////////////////////////////////////////////////////////////////
INLINE NxMat34 PhysxManager::
lMatrix4_to_nxMat34(const LMatrix4f &mat) {
  NxMat33 nxmat = lMatrix3_to_nxMat33(mat.get_upper_3());
  NxVec3 nxvec = lVecBase3_to_nxVec3(mat.get_row3( 3 ));
  return NxMat34(nxmat, nxvec);
}

////////////////////////////////////////////////////////////////////
//    Function : lQuaternion_to_nxQuat
//      Access : Public
// Description : convert LQuaternionf to NxQuat
////////////////////////////////////////////////////////////////////
INLINE NxQuat PhysxManager::
lQuaternion_to_nxQuat(const LQuaternionf &quat) {
  return NxQuat(NxVec3(quat.get_i(), quat.get_j(), quat.get_k()), quat.get_r());
}

