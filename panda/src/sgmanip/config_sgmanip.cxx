// Filename: config_sgmanip.cxx
// Created by:  drose (05Mar00)
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
