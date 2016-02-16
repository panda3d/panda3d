/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxForceFieldShapeDesc.cxx
 * @author enn0x
 * @date 2009-11-06
 */

#include "physxForceFieldShapeDesc.h"
#include "physxManager.h"

/**
 * Sets a possible debug name.
 */
void PhysxForceFieldShapeDesc::
set_name(const char *name) {

  _name = name ? name : "";
  ptr()->name = _name.c_str();
}

/**

 */
void PhysxForceFieldShapeDesc::
set_pos(const LPoint3f &pos) {

  nassertv(!pos.is_nan());
  ptr()->pose.t = PhysxManager::point3_to_nxVec3(pos);
}

/**

 */
void PhysxForceFieldShapeDesc::
set_mat(const LMatrix4f &mat) {

  nassertv(!mat.is_nan());
  ptr()->pose = PhysxManager::mat4_to_nxMat34(mat);
}

/**

 */
void PhysxForceFieldShapeDesc::
set_hpr(float h, float p, float r) {

  LQuaternionf q;
  LMatrix3f rot;
  NxMat34 m;

  q.set_hpr(LVector3f(h, p, r));
  q.extract_to_matrix(rot);

  ptr()->pose.M = PhysxManager::mat3_to_nxMat33(rot);
}

/**

 */
const char *PhysxForceFieldShapeDesc::
get_name() const {

  return ptr()->name;
}

/**

 */
LPoint3f PhysxForceFieldShapeDesc::
get_pos() const {

  return PhysxManager::nxVec3_to_point3(ptr()->pose.t);
}

/**

 */
LMatrix4f PhysxForceFieldShapeDesc::
get_mat() const {

  return PhysxManager::nxMat34_to_mat4(ptr()->pose);
}
