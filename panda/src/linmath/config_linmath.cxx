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

#include <dconfig.h>

Configure(config_linmath);
NotifyCategoryDef(linmath, "");

ConfigureFn(config_linmath) {
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
