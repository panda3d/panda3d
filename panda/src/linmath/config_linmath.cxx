// Filename: config_linmath.cxx
// Created by:  drose (23Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "config_linmath.h"
#include "luse.h"
#include "coordinateSystem.h"

#include "dconfig.h"

Configure(config_linmath);
NotifyCategoryDef(linmath, "");

ConfigureFn(config_linmath) {
  init_liblinmath();
}

ConfigVariableBool paranoid_hpr_quat
("paranoid-hpr-quat", false,
 "Set this true to doublecheck the quaternion-hpr compose and "
 "decompose operations against the quaternion-matrix and matrix-hpr "
 "operations.  This only has effect if NDEBUG is not defined.");

ConfigVariableBool temp_hpr_fix
("temp-hpr-fix", true,
 "Set this true to compute hpr's correctly.  Historically, Panda has "
 "applied these in the wrong order, and roll was backwards relative "
 "to the other two.  Set this false if you need compatibility with "
 "Panda's old hpr calculations.");

////////////////////////////////////////////////////////////////////
//     Function: init_liblinmath
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_liblinmath() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  LVecBase2f::init_type();
  LVecBase3f::init_type();
  LVecBase4f::init_type();
  LVector2f::init_type();
  LVector3f::init_type();
  LVector4f::init_type();
  LPoint2f::init_type();
  LPoint3f::init_type();
  LPoint4f::init_type();
  LMatrix3f::init_type();
  LMatrix4f::init_type();

  LVecBase2d::init_type();
  LVecBase3d::init_type();
  LVecBase4d::init_type();
  LVector2d::init_type();
  LVector3d::init_type();
  LVector4d::init_type();
  LPoint2d::init_type();
  LPoint3d::init_type();
  LPoint4d::init_type();
  LMatrix3d::init_type();
  LMatrix4d::init_type();

  LQuaternionf::init_type();
  LRotationf::init_type();
  LOrientationf::init_type();

  LQuaterniond::init_type();
  LRotationd::init_type();
  LOrientationd::init_type();
}
