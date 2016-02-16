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

////////////////////////////////////////////////////////////////////
//     Function: BulletRotationalLimitMotor::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BulletRotationalLimitMotor::
BulletRotationalLimitMotor(btRotationalLimitMotor &motor) 
 : _motor(motor) {

}

////////////////////////////////////////////////////////////////////
//     Function: BulletRotationalLimitMotor::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletRotationalLimitMotor::
BulletRotationalLimitMotor(const BulletRotationalLimitMotor &copy)
  : _motor(copy._motor) {

}

////////////////////////////////////////////////////////////////////
//     Function: BulletRotationalLimitMotor::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletRotationalLimitMotor::
~BulletRotationalLimitMotor() {

}

