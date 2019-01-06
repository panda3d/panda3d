/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_linmath.cxx
 * @author drose
 * @date 2000-02-23
 */

#include "config_linmath.h"
#include "luse.h"
#include "coordinateSystem.h"

#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_LINMATH)
  #error Buildsystem error: BUILDING_PANDA_LINMATH not defined
#endif

Configure(config_linmath);
NotifyCategoryDef(linmath, "");

ConfigureFn(config_linmath) {
  init_liblinmath();
}

ConfigVariableBool paranoid_hpr_quat
("paranoid-hpr-quat", false,
 PRC_DESC("Set this true to doublecheck the quaternion-hpr compose and "
          "decompose operations against the quaternion-matrix and matrix-hpr "
          "operations.  This only has effect if NDEBUG is not defined."));

ConfigVariableBool no_singular_invert
("no-singular-invert", false,
 PRC_DESC("Set this true to make singular-invert warning messages generate an "
          "assertion failure instead of just a warning (which can then be "
          "trapped with assert-abort)."));

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
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
  UnalignedLVecBase4f::init_type();
  UnalignedLMatrix4f::init_type();

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
  UnalignedLVecBase4d::init_type();
  UnalignedLMatrix4d::init_type();

  LVecBase2i::init_type();
  LVecBase3i::init_type();
  LVecBase4i::init_type();
  LVector2i::init_type();
  LVector3i::init_type();
  LVector4i::init_type();
  LPoint2i::init_type();
  LPoint3i::init_type();
  LPoint4i::init_type();
  UnalignedLVecBase4i::init_type();

  LQuaternionf::init_type();
  LRotationf::init_type();
  LOrientationf::init_type();

  LQuaterniond::init_type();
  LRotationd::init_type();
  LOrientationd::init_type();
}
