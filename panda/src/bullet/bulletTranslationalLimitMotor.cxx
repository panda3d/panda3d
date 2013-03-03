// Filename: bulletTranslationalLimitMotor.cxx
// Created by:  enn0x (03Mar13)
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

#include "bulletTranslationalLimitMotor.h"

////////////////////////////////////////////////////////////////////
//     Function: BulletTranslationalLimitMotor::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BulletTranslationalLimitMotor::
BulletTranslationalLimitMotor(btTranslationalLimitMotor &motor) 
 : _motor(motor) {

}

////////////////////////////////////////////////////////////////////
//     Function: BulletTranslationalLimitMotor::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletTranslationalLimitMotor::
BulletTranslationalLimitMotor(const BulletTranslationalLimitMotor &copy)
  : _motor(copy._motor) {

}

////////////////////////////////////////////////////////////////////
//     Function: BulletTranslationalLimitMotor::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletTranslationalLimitMotor::
~BulletTranslationalLimitMotor() {

}

