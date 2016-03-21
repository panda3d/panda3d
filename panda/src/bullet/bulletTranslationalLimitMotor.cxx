/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletTranslationalLimitMotor.cxx
 * @author enn0x
 * @date 2013-03-03
 */

#include "bulletTranslationalLimitMotor.h"

/**
 *
 */
BulletTranslationalLimitMotor::
BulletTranslationalLimitMotor(btTranslationalLimitMotor &motor)
 : _motor(motor) {

}

/**
 *
 */
BulletTranslationalLimitMotor::
BulletTranslationalLimitMotor(const BulletTranslationalLimitMotor &copy)
  : _motor(copy._motor) {

}

/**
 *
 */
BulletTranslationalLimitMotor::
~BulletTranslationalLimitMotor() {

}
