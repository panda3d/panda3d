// Filename: config_linmath.cxx
// Created by:  drose (23Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
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

// Set this true to doublecheck the quaternion-hpr compose and
// decompose operations against the quaternion-matrix and matrix-hpr
// operations.  This only has effect if NDEBUG is not defined.
const bool paranoid_hpr_quat = config_linmath.GetBool("paranoid-hpr-quat", false);

// Set this true to compute hpr's correctly.  Presently, we apply
// these in the wrong order, and roll is backwards relative to the
// other two.  But we can't globally fix this because some of our old
// tools, most notably egg-optchar, depend on the old broken behavior.
// Until we are able to rewrite these tools into the new system, we
// must keep the old behavior; setting this switch lets you use the
// new, correct behavior but you don't get animated characters.
const bool temp_hpr_fix = config_linmath.GetBool("temp-hpr-fix", false);

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

  string csstr = config_linmath.GetString("coordinate-system", "default");
  CoordinateSystem cs = parse_coordinate_system_string(csstr);

  if (cs == CS_invalid) {
    linmath_cat.error()
      << "Unexpected coordinate-system string: " << csstr << "\n";
    cs = CS_default;
  }
  default_coordinate_system = (cs == CS_default) ? CS_zup_right : cs;
}
