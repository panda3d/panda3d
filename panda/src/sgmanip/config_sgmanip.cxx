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

#include "nodePathBase.h"
#include "nodePath.h"
#include "nodePathLerps.h"

Configure(config_sgmanip);
NotifyCategoryDef(sgmanip, "");

ConfigureFn(config_sgmanip) {
  NodePathBase::init_type();
  NodePath::init_type();
  PosLerpFunctor::init_type();
  HprLerpFunctor::init_type();
  ScaleLerpFunctor::init_type();
  PosHprLerpFunctor::init_type();
  PosHprScaleLerpFunctor::init_type();
  ColorLerpFunctor::init_type();
  ColorScaleLerpFunctor::init_type();
}
