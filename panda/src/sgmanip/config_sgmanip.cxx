// Filename: config_sgmanip.cxx
// Created by:  drose (05Mar00)
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

#include "config_sgmanip.h"

#include <dconfig.h>

#include "nodePath.h"
#include "nodePathLerps.h"

Configure(config_sgmanip);
NotifyCategoryDef(sgmanip, "");

// Set this to true to generate an assertion failure whenever you
// first attempt to apply a singular transform matrix to the scene
// graph, or false to ignore this.  The default is true, which makes
// it much easier to track down singular matrix errors.  In NDEBUG
// mode, this is ignored and is effectively always false.
const bool check_singular_transform = config_sgmanip.GetBool("check-singular-transform", true);

ConfigureFn(config_sgmanip) {
  NodePath::init_type();
  PosLerpFunctor::init_type();
  HprLerpFunctor::init_type();
  ScaleLerpFunctor::init_type();
  PosHprLerpFunctor::init_type();
  HprScaleLerpFunctor::init_type();
  PosHprScaleLerpFunctor::init_type();
  ColorLerpFunctor::init_type();
  ColorScaleLerpFunctor::init_type();
}
