/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletSoftBodyControl.cxx
 * @author enn0x
 * @date 2010-03-04
 */

#include "bulletSoftBodyControl.h"

/**

 */
BulletSoftBodyControl::
BulletSoftBodyControl() {

  _goal = 0.0;
  _maxtorque = 0.0;

  _angle = 0.0;
  _sign = 0.0;
}

/**

 */
BulletSoftBodyControl::
~BulletSoftBodyControl() {

}

/**

 */
void BulletSoftBodyControl::
Prepare(btSoftBody::AJoint* joint) {

  if (btFabs(_sign) > 0.0) {
    joint->m_refs[0][0] = btCos(_angle * _sign);
    joint->m_refs[0][2] = btSin(_angle * _sign);
  }
}

/**

 */
btScalar BulletSoftBodyControl::
Speed(btSoftBody::AJoint *, btScalar current) {

  return (current + btMin(_maxtorque, btMax(-_maxtorque, _goal - current)));
}
