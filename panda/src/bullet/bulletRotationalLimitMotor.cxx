/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletRotationalLimitMotor.cxx
 * @author enn0x
 * @date 2013-03-03
 */

#include "bulletRotationalLimitMotor.h"

/**
 *
 */
BulletRotationalLimitMotor::
BulletRotationalLimitMotor(btRotationalLimitMotor &motor)
 : _motor(motor) {

}

/**
 *
 */
BulletRotationalLimitMotor::
BulletRotationalLimitMotor(const BulletRotationalLimitMotor &copy)
  : _motor(copy._motor) {

}

/**
 *
 */
BulletRotationalLimitMotor::
~BulletRotationalLimitMotor() {

}
