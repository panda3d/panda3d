/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bullet_utils.h
 * @author enn0x
 * @date 2010-01-23
 */

#ifndef __BULLET_UTILS_H__
#define __BULLET_UTILS_H__

#include "pandabase.h"

#include "bullet_includes.h"

#include "luse.h"
#include "pointerTo.h"
#include "pandaNode.h"
#include "nodePath.h"

// Conversion from Panda3D to Bullet
EXPCL_PANDABULLET btVector3 LVecBase3_to_btVector3(const LVecBase3 &v);
EXPCL_PANDABULLET btMatrix3x3 LMatrix3_to_btMatrix3x3(const LMatrix3 &m);
EXPCL_PANDABULLET btTransform LMatrix4_to_btTrans(const LMatrix4 &m);
EXPCL_PANDABULLET btQuaternion LQuaternion_to_btQuat(const LQuaternion &q);

// Conversion from Bullet to Panda3D
EXPCL_PANDABULLET LVecBase3 btVector3_to_LVecBase3(const btVector3 &v);
EXPCL_PANDABULLET LVector3 btVector3_to_LVector3(const btVector3 &v);
EXPCL_PANDABULLET LPoint3 btVector3_to_LPoint3(const btVector3 &p);
EXPCL_PANDABULLET LMatrix3 btMatrix3x3_to_LMatrix3(const btMatrix3x3 &m);
EXPCL_PANDABULLET LMatrix4 btTrans_to_LMatrix4(const btTransform &tf);
EXPCL_PANDABULLET LQuaternion btQuat_to_LQuaternion(const btQuaternion &q);

EXPCL_PANDABULLET CPT(TransformState) btTrans_to_TransformState(
  const btTransform &tf,
  const LVecBase3 &scale=LVecBase3(1.0f, 1.0f, 1.0f));

EXPCL_PANDABULLET btTransform TransformState_to_btTrans(
  CPT(TransformState) ts);

EXPCL_PANDABULLET void get_node_transform(btTransform &trans, PandaNode *node);

// UpAxis
BEGIN_PUBLISH

enum BulletUpAxis {
  X_up = 0,
  Y_up = 1,
  Z_up = 2,
};

EXPCL_PANDABULLET BulletUpAxis get_default_up_axis();
EXPCL_PANDABULLET int get_bullet_version();

END_PUBLISH

#include "bullet_utils.I"

#endif // __BULLET_UTILS_H__
